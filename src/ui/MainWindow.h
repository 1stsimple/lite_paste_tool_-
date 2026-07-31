#pragma once

#include <QMainWindow>
#include <QModelIndex>

#include "paste/PasteStrategy.h"

class QLineEdit;
class QListView;
class QSystemTrayIcon;
class QAction;
class QTimer;
class QCloseEvent;
class ClipboardRepository;
class ClipboardHistoryModel;

// 应用主窗口：历史列表 + 搜索框 + 系统托盘驻留。
// 关闭窗口时隐藏到托盘而非退出进程，符合"后台驻留"的产品定位。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ClipboardRepository *repository, QWidget *parent = nullptr);

    bool hasTrayIcon() const;

signals:
    void monitoringPausedChanged(bool paused);
    void quitRequested();
    // 窗口即将从隐藏变为可见前发出，用于在本窗口抢占焦点之前
    // 由外部（PasteController）记录当前系统前台窗口。
    void windowAboutToActivate();
    void pasteRequested(const QString &text, PasteMode mode);

public slots:
    // 切换主窗口显示/隐藏；由托盘菜单点击和全局快捷键共用，
    // 保证两条路径都会在窗口显示前发出 windowAboutToActivate。
    void toggleVisibility();

    // 供 PasteController::pasteFailed 连接，向用户提示粘贴失败原因。
    void onPasteFailed(const QString &reason);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onSearchTextEdited(const QString &text);
    void onSearchDebounceTimeout();
    void onTogglePauseTriggered();
    void onPinSelectedTriggered();
    void onDeleteSelectedTriggered();
    void onClearAllTriggered();
    void onNormalPasteTriggered();
    void onPlainTextPasteTriggered();
    void onItemDoubleClicked(const QModelIndex &index);
    void onRepositoryRejected(const QString &reason);
    void onPersistenceFailed(const QString &reason);

private:
    void setupUi();
    void setupTrayIcon();
    QIcon buildFallbackIcon() const;
    void requestPasteForRow(int row, PasteMode mode);

    ClipboardRepository *m_repository;
    ClipboardHistoryModel *m_model;

    QLineEdit *m_searchEdit;
    QListView *m_historyView;
    QTimer *m_searchDebounceTimer;

    QSystemTrayIcon *m_trayIcon;
    QAction *m_togglePauseAction;
    bool m_paused;
};
