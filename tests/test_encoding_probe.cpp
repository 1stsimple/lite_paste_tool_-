#include <QtTest>
#include <QDir>
#include <QFile>

#include "storage/FileHistoryStorage.h"

class EncodingProbeTest : public QObject
{
    Q_OBJECT

private slots:
    void chineseTextRoundTripsThroughFile();
};

void EncodingProbeTest::chineseTextRoundTripsThroughFile()
{
    const QString path = QDir::temp().filePath(QStringLiteral("litepaste_encoding_probe.json"));
    QFile::remove(path);

    FileHistoryStorage storage(path);

    std::vector<ClipboardItem> toSave;
    ClipboardItem item;
    item.id = "probe-1";
    item.text = QStringLiteral("安装方法");
    item.createdAt = QDateTime::currentDateTime();
    item.pinned = false;
    toSave.push_back(item);

    QString saveError;
    QVERIFY2(storage.save(toSave, &saveError), qPrintable(saveError));

    std::vector<ClipboardItem> loaded;
    QString loadError;
    QVERIFY2(storage.load(loaded, &loadError), qPrintable(loadError));

    QCOMPARE(loaded.size(), static_cast<size_t>(1));
    QCOMPARE(loaded.front().text, QStringLiteral("安装方法"));

    QFile::remove(path);
}

QTEST_APPLESS_MAIN(EncodingProbeTest)
#include "test_encoding_probe.moc"
