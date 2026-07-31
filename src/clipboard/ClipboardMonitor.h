#pragma once

#include <QObject>
#include <QString>

class QClipboard;

// 监听系统文本剪贴板变化。
// 职责：订阅 QClipboard::dataChanged 事件（非轮询），过滤非文本内容，
//       对连续重复内容去重，并在暂停状态下忽略变化。
// 输入：无（被动响应系统剪贴板事件）。
// 输出：textCaptured 信号，携带新捕获的文本。
// 边界条件：剪贴板内容为空、非文本（图片/文件）、与上一条完全相同时不发出信号。
// 失败行为：无法访问剪贴板时发出 accessFailed 信号，不静默吞掉错误。
class ClipboardMonitor : public QObject
{
    Q_OBJECT

public:
    explicit ClipboardMonitor(QObject *parent = nullptr);

    bool isPaused() const;
    void setPaused(bool paused);

    // 程序自身写回剪贴板前调用，忽略紧随其后的一次 dataChanged，
    // 避免粘贴恢复原剪贴板内容时被重新记录为历史。
    void ignoreNextChange();

signals:
    void textCaptured(const QString &text);
    void accessFailed(const QString &reason);

private slots:
    void handleClipboardChanged();

private:
    QClipboard *m_clipboard;
    QString m_lastText;
    bool m_paused;
    bool m_ignoreNext;
};
