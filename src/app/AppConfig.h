#pragma once

#include <QString>

class QSettings;

// 应用配置模块：基于 QSettings 持久化用户可调整的行为参数。
// 所有 getter 在未持久化时返回合理默认值，setter 立即写入磁盘。
class AppConfig
{
public:
    AppConfig();
    virtual ~AppConfig();

    // 声明为 virtual 以便单元测试用内存实现替换真实 QSettings 读写。
    virtual int maxHistoryCount() const;
    void setMaxHistoryCount(int count);

    // 单条历史记录允许保存的最大字符数，超过此长度的复制内容不会被记录。
    virtual int maxItemTextLength() const;
    void setMaxItemTextLength(int length);

    // 历史记录保留天数，0 表示不自动过期。
    virtual int retentionDays() const;
    void setRetentionDays(int days);

    // 粘贴完成后是否恢复用户原来的剪贴板内容。
    virtual bool restoreClipboardAfterPaste() const;
    void setRestoreClipboardAfterPaste(bool enabled);

    QString historyFilePath() const;

private:
    QSettings *m_settings;
};
