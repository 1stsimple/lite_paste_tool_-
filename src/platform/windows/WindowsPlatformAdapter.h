#pragma once

#include <QAbstractNativeEventFilter>
#include <QSet>

#include "platform/PlatformAdapter.h"

// PlatformAdapter 的 Windows 实现。
// 全局快捷键基于 RegisterHotKey + WM_HOTKEY 消息（通过原生事件过滤器捕获）；
// 按键模拟基于 SetForegroundWindow + SendInput 模拟 Ctrl+V。
class WindowsPlatformAdapter : public PlatformAdapter, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit WindowsPlatformAdapter(QObject *parent = nullptr);
    ~WindowsPlatformAdapter() override;

    bool registerHotkey(int id, Qt::KeyboardModifiers modifiers, Qt::Key key) override;
    void unregisterHotkey(int id) override;
    WindowHandle activeWindow() const override;
    bool sendPasteKeystroke(WindowHandle target) override;

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

private:
    QSet<int> m_registeredIds;
};
