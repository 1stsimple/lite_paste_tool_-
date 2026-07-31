#include "ClipboardMonitor.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

ClipboardMonitor::ClipboardMonitor(QObject *parent)
    : QObject(parent)
    , m_clipboard(QApplication::clipboard())
    , m_paused(false)
    , m_ignoreNext(false)
{
    if (!m_clipboard) {
        emit accessFailed(QStringLiteral("无法获取系统剪贴板对象"));
        return;
    }
    connect(m_clipboard, &QClipboard::dataChanged, this, &ClipboardMonitor::handleClipboardChanged);
}

bool ClipboardMonitor::isPaused() const
{
    return m_paused;
}

void ClipboardMonitor::setPaused(bool paused)
{
    m_paused = paused;
}

void ClipboardMonitor::ignoreNextChange()
{
    m_ignoreNext = true;
}

void ClipboardMonitor::handleClipboardChanged()
{
    if (m_ignoreNext) {
        m_ignoreNext = false;
        return;
    }

    if (m_paused) {
        return;
    }

    const QMimeData *mimeData = m_clipboard->mimeData();
    if (!mimeData) {
        emit accessFailed(QStringLiteral("读取剪贴板 mimeData 失败"));
        return;
    }

    if (!mimeData->hasText()) {
        return;
    }

    const QString text = mimeData->text();
    if (text.isEmpty()) {
        return;
    }

    if (text == m_lastText) {
        return;
    }

    m_lastText = text;
    emit textCaptured(text);
}
