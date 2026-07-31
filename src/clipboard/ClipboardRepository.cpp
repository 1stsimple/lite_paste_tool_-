#include "ClipboardRepository.h"

#include <algorithm>

#include <QTimer>
#include <QUuid>
#include <QtConcurrent/QtConcurrent>

#include "app/AppConfig.h"
#include "storage/HistoryStorage.h"

namespace
{
const int kSaveDebounceMs = 1500;
}

ClipboardRepository::ClipboardRepository(AppConfig *config, HistoryStorage *storage, QObject *parent)
    : QObject(parent)
    , m_config(config)
    , m_storage(storage)
    , m_saveDebounceTimer(new QTimer(this))
    , m_saveWatcher(new QFutureWatcher<QString>(this))
    , m_saveRequestedAgain(false)
{
    m_saveDebounceTimer->setSingleShot(true);
    connect(m_saveDebounceTimer, &QTimer::timeout, this, &ClipboardRepository::onSaveDebounced);
    connect(m_saveWatcher, &QFutureWatcher<QString>::finished, this, &ClipboardRepository::onSaveFinished);
}

void ClipboardRepository::loadFromStorage()
{
    QString error;
    if (!m_storage->load(m_items, &error)) {
        emit persistenceFailed(error);
        return;
    }
    reorder();
    emit itemsReset();
}

const std::vector<ClipboardItem> &ClipboardRepository::items() const
{
    return m_items;
}

std::vector<ClipboardItem> ClipboardRepository::search(const QString &keyword) const
{
    if (keyword.isEmpty()) {
        return m_items;
    }

    std::vector<ClipboardItem> result;
    for (const ClipboardItem &item : m_items) {
        if (item.text.contains(keyword, Qt::CaseInsensitive)) {
            result.push_back(item);
        }
    }
    return result;
}

void ClipboardRepository::addText(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }

    const int maxLength = m_config->maxItemTextLength();
    if (text.length() > maxLength) {
        emit itemRejected(QStringLiteral("内容超过 %1 字符上限，未记录").arg(maxLength));
        return;
    }

    // 与上一次捕获内容相同时，只更新时间戳并将其视为"仍是最新"，不新增条目。
    if (!m_lastCapturedText.isEmpty() && text == m_lastCapturedText) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [this](const ClipboardItem &item) { return item.id == m_lastCapturedId; });
        if (it != m_items.end()) {
            it->createdAt = QDateTime::currentDateTime();
            emit itemsReset();
            scheduleSave();
            return;
        }
        // 原条目已被用户删除，走新增逻辑。
    }

    ClipboardItem item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    item.text = text;
    item.createdAt = QDateTime::currentDateTime();
    item.pinned = false;

    // 新条目插入最前，保持"未置顶部分按新→旧排列"的不变量，
    // 不依赖 QDateTime 的毫秒级精度做排序，避免同一毫秒内多次复制时排序不稳定。
    m_items.insert(m_items.begin(), item);
    m_lastCapturedText = text;
    m_lastCapturedId = item.id;

    reorder();
    enforceLimits();
    emit itemsReset();
    scheduleSave();
}

void ClipboardRepository::removeItem(const std::string &id)
{
    const size_t before = m_items.size();
    m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
                      [&id](const ClipboardItem &item) { return item.id == id; }),
        m_items.end());

    if (m_items.size() != before) {
        emit itemsReset();
        scheduleSave();
    }
}

void ClipboardRepository::togglePinned(const std::string &id)
{
    for (ClipboardItem &item : m_items) {
        if (item.id == id) {
            item.pinned = !item.pinned;
            reorder();
            emit itemsReset();
            scheduleSave();
            return;
        }
    }
}

void ClipboardRepository::clear()
{
    if (m_items.empty()) {
        return;
    }
    m_items.clear();
    m_lastCapturedText.clear();
    m_lastCapturedId.clear();
    emit itemsReset();
    scheduleSave();
}

void ClipboardRepository::reorder()
{
    // 置顶项浮动到最前面；stable_partition 保证同组内原有的新→旧相对顺序不变。
    std::stable_partition(m_items.begin(), m_items.end(), [](const ClipboardItem &item) { return item.pinned; });
}

void ClipboardRepository::enforceLimits()
{
    const int maxCount = m_config->maxHistoryCount();
    const int retentionDays = m_config->retentionDays();

    if (retentionDays > 0) {
        const QDateTime cutoff = QDateTime::currentDateTime().addDays(-retentionDays);
        m_items.erase(std::remove_if(m_items.begin(), m_items.end(),
                          [&cutoff](const ClipboardItem &item) {
                              return !item.pinned && item.createdAt < cutoff;
                          }),
            m_items.end());
    }

    // m_items 已保持"置顶在前，未置顶按新→旧"的顺序，直接从后往前数超出上限的未置顶项即可。
    int unpinnedSeen = 0;
    std::vector<ClipboardItem> kept;
    kept.reserve(m_items.size());
    for (const ClipboardItem &item : m_items) {
        if (item.pinned) {
            kept.push_back(item);
            continue;
        }
        if (unpinnedSeen < maxCount) {
            kept.push_back(item);
            ++unpinnedSeen;
        }
    }
    m_items.swap(kept);
}

void ClipboardRepository::scheduleSave()
{
    m_saveDebounceTimer->start(kSaveDebounceMs);
}

void ClipboardRepository::onSaveDebounced()
{
    if (m_saveWatcher->isRunning()) {
        // 上一次写入尚未完成，等它结束后由 onSaveFinished 触发补写。
        m_saveRequestedAgain = true;
        return;
    }

    HistoryStorage *storage = m_storage;
    std::vector<ClipboardItem> snapshot = m_items;

    QFuture<QString> future = QtConcurrent::run([storage, snapshot]() -> QString {
        QString error;
        storage->save(snapshot, &error);
        return error;
    });
    m_saveWatcher->setFuture(future);
}

void ClipboardRepository::onSaveFinished()
{
    const QString error = m_saveWatcher->result();
    if (!error.isEmpty()) {
        emit persistenceFailed(error);
    }

    if (m_saveRequestedAgain) {
        m_saveRequestedAgain = false;
        onSaveDebounced();
    }
}
