#pragma once

#include <vector>

#include <QObject>
#include <QString>
#include <QFutureWatcher>

#include "ClipboardItem.h"

class AppConfig;
class HistoryStorage;
class QTimer;

// 剪贴板历史记录的内存仓库，是历史数据的唯一权威来源。
// 职责：接收新捕获的文本、去重合并、容量与过期清理、置顶管理、
//       委托 HistoryStorage 做持久化（写入在后台线程执行）。
// 输入：ClipboardMonitor 捕获的文本（通过 addText）。
// 输出：itemsReset 信号驱动 UI 层的 Model/View 刷新。
// 边界条件：空文本、超长文本、与最近一条完全相同的文本、空历史。
// 失败行为：持久化失败通过 persistenceFailed 信号上报，不静默吞掉。
class ClipboardRepository : public QObject
{
    Q_OBJECT

public:
    ClipboardRepository(AppConfig *config, HistoryStorage *storage, QObject *parent = nullptr);

    // 启动时从持久化介质同步加载一次，UI 显示前调用。
    void loadFromStorage();

    const std::vector<ClipboardItem> &items() const;

    std::vector<ClipboardItem> search(const QString &keyword) const;

public slots:
    void addText(const QString &text);
    void removeItem(const std::string &id);
    void togglePinned(const std::string &id);
    void clear();

signals:
    void itemsReset();
    void itemRejected(const QString &reason);
    void persistenceFailed(const QString &reason);

private slots:
    void onSaveDebounced();
    void onSaveFinished();

private:
    void reorder();
    void enforceLimits();
    void scheduleSave();

    AppConfig *m_config;
    HistoryStorage *m_storage;
    std::vector<ClipboardItem> m_items;
    QString m_lastCapturedText;
    std::string m_lastCapturedId;
    QTimer *m_saveDebounceTimer;
    QFutureWatcher<QString> *m_saveWatcher;
    bool m_saveRequestedAgain;
};
