#pragma once

#include <map>
#include <memory>

#include <QObject>
#include <QString>

#include "PasteStrategy.h"
#include "platform/PlatformAdapter.h"

class AppConfig;
class ClipboardMonitor;

// 粘贴流程的统一调度者：把历史文本按指定 PasteMode 转换后写入系统剪贴板，
// 再交给 PlatformAdapter 完成"切回目标窗口 + 模拟按键"。
// 输入：待粘贴原始文本、PasteMode；触发前必须先调用 captureTargetWindow()
//       记录用户切换到本程序之前的活动窗口。
// 边界条件：未捕获目标窗口、空文本、该模式尚未实现的策略。
// 失败行为：一律通过 pasteFailed 信号上报，不静默吞掉。
class PasteController : public QObject
{
    Q_OBJECT

public:
    PasteController(PlatformAdapter *platform, ClipboardMonitor *monitor, AppConfig *config, QObject *parent = nullptr);

public slots:
    // 记录当前系统前台窗口，供后续 pasteText 使用。应在本程序窗口
    // 即将获得焦点之前调用，避免捕获到自己的窗口。
    void captureTargetWindow();

    bool pasteText(const QString &rawText, PasteMode mode);

signals:
    void pasteFailed(const QString &reason);

private:
    PlatformAdapter *m_platform;
    ClipboardMonitor *m_monitor;
    AppConfig *m_config;
    std::map<PasteMode, std::unique_ptr<PasteStrategy>> m_strategies;
    WindowHandle m_targetWindow;
};
