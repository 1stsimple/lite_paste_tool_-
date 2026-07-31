#include <QtTest>

#include "app/AppConfig.h"
#include "clipboard/ClipboardRepository.h"
#include "storage/HistoryStorage.h"

namespace
{

// 用固定值覆盖真实 QSettings 读写，避免单元测试污染用户配置文件。
class TestAppConfig : public AppConfig
{
public:
    int maxHistoryCount() const override { return m_maxHistoryCount; }
    int maxItemTextLength() const override { return m_maxItemTextLength; }
    int retentionDays() const override { return m_retentionDays; }

    int m_maxHistoryCount = 1000;
    int m_maxItemTextLength = 200 * 1024;
    int m_retentionDays = 0;
};

// 内存实现，避免测试触碰磁盘。
class InMemoryHistoryStorage : public HistoryStorage
{
public:
    bool load(std::vector<ClipboardItem> &items, QString * /*errorMessage*/ = nullptr) override
    {
        items = m_items;
        return true;
    }

    bool save(const std::vector<ClipboardItem> &items, QString * /*errorMessage*/ = nullptr) override
    {
        m_items = items;
        return true;
    }

    std::vector<ClipboardItem> m_items;
};

} // namespace

class ClipboardRepositoryTest : public QObject
{
    Q_OBJECT

private slots:
    void consecutiveDuplicateIsMerged();
    void capacityLimitEvictsOldestUnpinned();
    void pinnedItemsSurviveCapacityLimit();
    void oversizedTextIsRejected();
    void removeAndClearWork();
};

void ClipboardRepositoryTest::consecutiveDuplicateIsMerged()
{
    TestAppConfig config;
    InMemoryHistoryStorage storage;
    ClipboardRepository repo(&config, &storage, nullptr);

    repo.addText("hello");
    repo.addText("hello");
    repo.addText("hello");

    QCOMPARE(repo.items().size(), static_cast<size_t>(1));
    QCOMPARE(repo.items().front().text, QString("hello"));
}

void ClipboardRepositoryTest::capacityLimitEvictsOldestUnpinned()
{
    TestAppConfig config;
    config.m_maxHistoryCount = 3;
    InMemoryHistoryStorage storage;
    ClipboardRepository repo(&config, &storage, nullptr);

    repo.addText("first");
    repo.addText("second");
    repo.addText("third");
    repo.addText("fourth");
    repo.addText("fifth");

    QCOMPARE(repo.items().size(), static_cast<size_t>(3));
    // 最新插入的三条应被保留，最旧的两条被淘汰。
    QCOMPARE(repo.items()[0].text, QString("fifth"));
    QCOMPARE(repo.items()[1].text, QString("fourth"));
    QCOMPARE(repo.items()[2].text, QString("third"));
}

void ClipboardRepositoryTest::pinnedItemsSurviveCapacityLimit()
{
    TestAppConfig config;
    config.m_maxHistoryCount = 2;
    InMemoryHistoryStorage storage;
    ClipboardRepository repo(&config, &storage, nullptr);

    repo.addText("keep-pinned");
    const std::string pinnedId = repo.items().front().id;
    repo.togglePinned(pinnedId);

    repo.addText("a");
    repo.addText("b");
    repo.addText("c");

    // 置顶项不计入容量上限，应始终存在。
    bool foundPinned = false;
    int unpinnedCount = 0;
    for (const ClipboardItem &item : repo.items()) {
        if (item.id == pinnedId) {
            foundPinned = true;
            QVERIFY(item.pinned);
        } else {
            ++unpinnedCount;
        }
    }
    QVERIFY(foundPinned);
    QCOMPARE(unpinnedCount, 2);
}

void ClipboardRepositoryTest::oversizedTextIsRejected()
{
    TestAppConfig config;
    config.m_maxItemTextLength = 5;
    InMemoryHistoryStorage storage;
    ClipboardRepository repo(&config, &storage, nullptr);

    QSignalSpy rejectedSpy(&repo, &ClipboardRepository::itemRejected);

    repo.addText("this text is way too long");

    QCOMPARE(repo.items().size(), static_cast<size_t>(0));
    QCOMPARE(rejectedSpy.count(), 1);
}

void ClipboardRepositoryTest::removeAndClearWork()
{
    TestAppConfig config;
    InMemoryHistoryStorage storage;
    ClipboardRepository repo(&config, &storage, nullptr);

    repo.addText("one");
    repo.addText("two");
    QCOMPARE(repo.items().size(), static_cast<size_t>(2));

    const std::string idToRemove = repo.items().back().id;
    repo.removeItem(idToRemove);
    QCOMPARE(repo.items().size(), static_cast<size_t>(1));

    repo.clear();
    QCOMPARE(repo.items().size(), static_cast<size_t>(0));
}

QTEST_MAIN(ClipboardRepositoryTest)
#include "test_clipboard_repository.moc"
