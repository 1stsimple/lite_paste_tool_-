// _WIN32_WINNT 必须在本翻译单元的任何头文件（包括 Qt 头文件）被包含之前定义，
// 否则 <sdkddkver.h> 会先按默认值把它锁定，导致 MOD_NOREPEAT 等 Windows 7+
// 常量在后面 #include <Windows.h> 时仍不可见。
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 // Windows 7+
#endif
#define WIN32_LEAN_AND_MEAN

#include "WindowsPlatformAdapter.h"

#include <QCoreApplication>

#include <Windows.h>

namespace
{
// 仅字母(A-Z)与数字(0-9)键：Qt::Key_A..Z / Key_0..9 的数值与对应的
// Windows 虚拟键码(VK_A..Z / VK_0..9)完全一致，可直接转换。
bool tryConvertToVirtualKey(Qt::Key key, UINT *outVk)
{
    if ((key >= Qt::Key_A && key <= Qt::Key_Z) || (key >= Qt::Key_0 && key <= Qt::Key_9)) {
        *outVk = static_cast<UINT>(key);
        return true;
    }
    return false;
}
}

WindowsPlatformAdapter::WindowsPlatformAdapter(QObject *parent)
    : PlatformAdapter(parent)
{
    qApp->installNativeEventFilter(this);
}

WindowsPlatformAdapter::~WindowsPlatformAdapter()
{
    for (int id : m_registeredIds) {
        ::UnregisterHotKey(nullptr, id);
    }
    qApp->removeNativeEventFilter(this);
}

bool WindowsPlatformAdapter::registerHotkey(int id, Qt::KeyboardModifiers modifiers, Qt::Key key)
{
    UINT vk = 0;
    if (!tryConvertToVirtualKey(key, &vk)) {
        emit error(QStringLiteral("暂不支持将该按键注册为全局快捷键"));
        return false;
    }

    UINT mod = MOD_NOREPEAT;
    if (modifiers & Qt::ControlModifier) {
        mod |= MOD_CONTROL;
    }
    if (modifiers & Qt::AltModifier) {
        mod |= MOD_ALT;
    }
    if (modifiers & Qt::ShiftModifier) {
        mod |= MOD_SHIFT;
    }
    if (modifiers & Qt::MetaModifier) {
        mod |= MOD_WIN;
    }

    if (m_registeredIds.contains(id)) {
        ::UnregisterHotKey(nullptr, id);
        m_registeredIds.remove(id);
    }

    if (!::RegisterHotKey(nullptr, id, mod, vk)) {
        const DWORD errorCode = ::GetLastError();
        emit error(QStringLiteral("注册全局快捷键失败（错误码 %1），可能已被其他程序占用").arg(errorCode));
        return false;
    }

    m_registeredIds.insert(id);
    return true;
}

void WindowsPlatformAdapter::unregisterHotkey(int id)
{
    if (m_registeredIds.remove(id)) {
        ::UnregisterHotKey(nullptr, id);
    }
}

WindowHandle WindowsPlatformAdapter::activeWindow() const
{
    return reinterpret_cast<WindowHandle>(::GetForegroundWindow());
}

bool WindowsPlatformAdapter::sendPasteKeystroke(WindowHandle target)
{
    HWND hwnd = reinterpret_cast<HWND>(target);
    if (!hwnd || !::IsWindow(hwnd)) {
        emit error(QStringLiteral("目标窗口已失效，无法发送粘贴指令"));
        return false;
    }

    const DWORD targetThreadId = ::GetWindowThreadProcessId(hwnd, nullptr);
    const DWORD currentThreadId = ::GetCurrentThreadId();
    const bool needsAttach = targetThreadId != 0 && targetThreadId != currentThreadId;

    // 附加输入队列是让 SetForegroundWindow 在目标非当前前台线程时依然生效的标准做法。
    if (needsAttach) {
        ::AttachThreadInput(currentThreadId, targetThreadId, TRUE);
    }
    ::SetForegroundWindow(hwnd);
    ::BringWindowToTop(hwnd);
    if (needsAttach) {
        ::AttachThreadInput(currentThreadId, targetThreadId, FALSE);
    }

    // 给目标窗口短暂时间完成焦点切换，避免按键过早发送而落到错误的窗口上。
    ::Sleep(30);

    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = static_cast<WORD>('V');
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = static_cast<WORD>('V');
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    const UINT sent = ::SendInput(4, inputs, sizeof(INPUT));
    if (sent != 4) {
        emit error(QStringLiteral("模拟按键发送失败（错误码 %1）").arg(::GetLastError()));
        return false;
    }
    return true;
}

bool WindowsPlatformAdapter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    if (eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG") {
        return false;
    }

    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_HOTKEY) {
        emit hotkeyTriggered(static_cast<int>(msg->wParam));
    }
    return false;
}

std::unique_ptr<PlatformAdapter> createPlatformAdapter(QObject *parent)
{
    return std::unique_ptr<PlatformAdapter>(new WindowsPlatformAdapter(parent));
}
