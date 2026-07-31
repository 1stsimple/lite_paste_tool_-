#include "Application.h"

#include <QCoreApplication>
#include <QDebug>

#include "AppConfig.h"
#include "storage/FileHistoryStorage.h"
#include "clipboard/ClipboardMonitor.h"
#include "clipboard/ClipboardRepository.h"
#include "platform/PlatformAdapter.h"
#include "paste/PasteController.h"
#include "ui/MainWindow.h"

namespace
{
// 全局快捷键 id 由本进程内自行分配，只需保证唯一。
const int kToggleWindowHotkeyId = 1;
}

Application::Application(QObject *parent)
    : QObject(parent)
{
}

Application::~Application() = default;

void Application::start()
{
    m_config.reset(new AppConfig());
    m_storage.reset(new FileHistoryStorage(m_config->historyFilePath()));
    m_repository.reset(new ClipboardRepository(m_config.get(), m_storage.get(), this));
    m_monitor.reset(new ClipboardMonitor(this));
    m_platformAdapter = createPlatformAdapter(this);
    m_pasteController.reset(new PasteController(m_platformAdapter.get(), m_monitor.get(), m_config.get(), this));
    m_mainWindow.reset(new MainWindow(m_repository.get()));

    connect(m_monitor.get(), &ClipboardMonitor::textCaptured, m_repository.get(), &ClipboardRepository::addText);
    connect(m_monitor.get(), &ClipboardMonitor::accessFailed, this, [](const QString &reason) {
        qWarning() << "ClipboardMonitor access failed:" << reason;
    });

    connect(m_mainWindow.get(), &MainWindow::monitoringPausedChanged, m_monitor.get(), &ClipboardMonitor::setPaused);
    connect(m_mainWindow.get(), &MainWindow::quitRequested, this, []() { QCoreApplication::quit(); });
    connect(m_mainWindow.get(), &MainWindow::windowAboutToActivate, m_pasteController.get(), &PasteController::captureTargetWindow);
    connect(m_mainWindow.get(), &MainWindow::pasteRequested, m_pasteController.get(), &PasteController::pasteText);
    connect(m_pasteController.get(), &PasteController::pasteFailed, m_mainWindow.get(), &MainWindow::onPasteFailed);

    connect(m_platformAdapter.get(), &PlatformAdapter::hotkeyTriggered, this, [this](int id) {
        if (id == kToggleWindowHotkeyId) {
            m_mainWindow->toggleVisibility();
        }
    });
    connect(m_platformAdapter.get(), &PlatformAdapter::error, this, [](const QString &reason) {
        qWarning() << "PlatformAdapter error:" << reason;
    });

    if (!m_platformAdapter->registerHotkey(kToggleWindowHotkeyId, Qt::ControlModifier | Qt::AltModifier, Qt::Key_V)) {
        qWarning() << "Failed to register global hotkey Ctrl+Alt+V for toggling main window";
    }

    m_repository->loadFromStorage();

    // 有托盘图标时默认后台驻留，靠热键/托盘唤出；否则没有其他打开入口，直接显示窗口。
    if (m_mainWindow->hasTrayIcon()) {
        m_mainWindow->hide();
    } else {
        m_mainWindow->show();
    }
}
