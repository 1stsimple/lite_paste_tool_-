#include "PasteController.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTimer>

#include "app/AppConfig.h"
#include "clipboard/ClipboardMonitor.h"

namespace
{
const int kRestoreClipboardDelayMs = 300;
}

PasteController::PasteController(PlatformAdapter *platform, ClipboardMonitor *monitor, AppConfig *config, QObject *parent)
    : QObject(parent)
    , m_platform(platform)
    , m_monitor(monitor)
    , m_config(config)
    , m_targetWindow(nullptr)
{
    m_strategies[PasteMode::Normal] = createNormalPasteStrategy();
    m_strategies[PasteMode::PlainText] = createPlainTextPasteStrategy();
}

void PasteController::captureTargetWindow()
{
    m_targetWindow = m_platform->activeWindow();
}

bool PasteController::pasteText(const QString &rawText, PasteMode mode)
{
    auto strategyIt = m_strategies.find(mode);
    if (strategyIt == m_strategies.end()) {
        emit pasteFailed(QStringLiteral("该粘贴模式尚未实现"));
        return false;
    }

    const QString text = strategyIt->second->transform(rawText);
    if (text.isEmpty()) {
        emit pasteFailed(QStringLiteral("内容为空，未执行粘贴"));
        return false;
    }

    if (!m_targetWindow) {
        emit pasteFailed(QStringLiteral("未捕获到目标窗口，请先切换到目标程序后再触发粘贴"));
        return false;
    }

    QClipboard *clipboard = QApplication::clipboard();
    QString originalText;
    bool hadOriginalText = false;
    if (m_config->restoreClipboardAfterPaste()) {
        const QMimeData *originalMime = clipboard->mimeData();
        if (originalMime && originalMime->hasText()) {
            originalText = originalMime->text();
            hadOriginalText = true;
        }
    }

    m_monitor->ignoreNextChange();
    clipboard->setText(text);

    if (!m_platform->sendPasteKeystroke(m_targetWindow)) {
        return false; // PlatformAdapter 已发出 error 信号说明原因。
    }

    if (m_config->restoreClipboardAfterPaste() && hadOriginalText) {
        ClipboardMonitor *monitor = m_monitor;
        QTimer::singleShot(kRestoreClipboardDelayMs, this, [monitor, originalText]() {
            monitor->ignoreNextChange();
            QApplication::clipboard()->setText(originalText);
        });
    }

    return true;
}
