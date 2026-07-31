#pragma once

#include <memory>

#include <QObject>
#include <QString>
#include <Qt>

// 不透明的原生窗口句柄（Windows 上是 HWND），只允许原样保存和传回，
// 业务代码不应解释其内部值。
using WindowHandle = void *;

// 平台相关能力的统一接口：全局快捷键、活动窗口识别、按键模拟。
// 所有实现必须放在 platform/<平台名>/ 目录下，业务代码只依赖此接口，
// 不得直接调用具体系统 API。
// 失败行为：注册/发送失败必须通过 error 信号上报，不允许静默吞掉。
class PlatformAdapter : public QObject
{
    Q_OBJECT

public:
    explicit PlatformAdapter(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~PlatformAdapter() override = default;

    // 注册一个全局快捷键，id 由调用方分配并需保证唯一。
    // 仅支持字母、数字键与 Ctrl/Alt/Shift/Meta 组合修饰键。
    // 成功返回 true；失败（如已被其他程序占用）返回 false 并发出 error 信号。
    virtual bool registerHotkey(int id, Qt::KeyboardModifiers modifiers, Qt::Key key) = 0;

    virtual void unregisterHotkey(int id) = 0;

    // 当前系统前台窗口句柄；失败或不可用时返回 nullptr。
    virtual WindowHandle activeWindow() const = 0;

    // 将焦点切换到 target 并模拟发送一次 Ctrl+V。
    // target 为 nullptr 或已失效的窗口句柄时返回 false 并发出 error 信号。
    virtual bool sendPasteKeystroke(WindowHandle target) = 0;

signals:
    void hotkeyTriggered(int id);
    void error(const QString &reason);
};

// 按当前编译目标平台创建对应的 PlatformAdapter 实现。
std::unique_ptr<PlatformAdapter> createPlatformAdapter(QObject *parent = nullptr);
