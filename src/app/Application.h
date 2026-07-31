#pragma once

#include <memory>

#include <QObject>

class AppConfig;
class HistoryStorage;
class ClipboardMonitor;
class ClipboardRepository;
class MainWindow;
class PlatformAdapter;
class PasteController;

// 应用程序组合根：创建并连接各核心模块的生命周期，
// 不包含具体业务逻辑，只负责装配与信号槽转发。
class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override;

    // 完成模块装配、加载历史记录、显示托盘图标。
    void start();

private:
    std::unique_ptr<AppConfig> m_config;
    std::unique_ptr<HistoryStorage> m_storage;
    std::unique_ptr<ClipboardRepository> m_repository;
    std::unique_ptr<ClipboardMonitor> m_monitor;
    std::unique_ptr<PlatformAdapter> m_platformAdapter;
    std::unique_ptr<PasteController> m_pasteController;
    std::unique_ptr<MainWindow> m_mainWindow;
};
