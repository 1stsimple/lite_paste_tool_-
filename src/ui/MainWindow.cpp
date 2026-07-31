#include "MainWindow.h"

#include <QAction>
#include <QCloseEvent>
#include <QLineEdit>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QStatusBar>

#include "clipboard/ClipboardRepository.h"
#include "ClipboardHistoryModel.h"

namespace
{
const int kSearchDebounceMs = 250;
}

MainWindow::MainWindow(ClipboardRepository *repository, QWidget *parent)
    : QMainWindow(parent)
    , m_repository(repository)
    , m_model(new ClipboardHistoryModel(repository, this))
    , m_searchEdit(nullptr)
    , m_historyView(nullptr)
    , m_searchDebounceTimer(new QTimer(this))
    , m_trayIcon(nullptr)
    , m_togglePauseAction(nullptr)
    , m_paused(false)
{
    m_searchDebounceTimer->setSingleShot(true);
    connect(m_searchDebounceTimer, &QTimer::timeout, this, &MainWindow::onSearchDebounceTimeout);

    connect(m_repository, &ClipboardRepository::itemRejected, this, &MainWindow::onRepositoryRejected);
    connect(m_repository, &ClipboardRepository::persistenceFailed, this, &MainWindow::onPersistenceFailed);

    setupUi();
    setupTrayIcon();

    setWindowTitle(QStringLiteral("LitePaste"));
    resize(420, 560);
}

bool MainWindow::hasTrayIcon() const
{
    return m_trayIcon != nullptr;
}

void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(central);
    m_searchEdit->setPlaceholderText(QStringLiteral("搜索历史记录…"));
    connect(m_searchEdit, &QLineEdit::textEdited, this, &MainWindow::onSearchTextEdited);

    QPushButton *clearButton = new QPushButton(QStringLiteral("清空"), central);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearAllTriggered);

    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(clearButton);

    m_historyView = new QListView(central);
    m_historyView->setModel(m_model);
    m_historyView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_historyView->setAlternatingRowColors(true);
    connect(m_historyView, &QListView::doubleClicked, this, &MainWindow::onItemDoubleClicked);
    connect(m_historyView, &QListView::customContextMenuRequested, this, [this](const QPoint &pos) {
        const QModelIndex index = m_historyView->indexAt(pos);
        if (!index.isValid()) {
            return;
        }
        m_historyView->setCurrentIndex(index);

        QMenu menu(this);
        QAction *normalPasteAction = menu.addAction(QStringLiteral("普通粘贴"));
        QAction *plainTextPasteAction = menu.addAction(QStringLiteral("纯文本粘贴"));
        menu.addSeparator();
        const bool pinned = index.data(ClipboardHistoryModel::PinnedRole).toBool();
        QAction *pinAction = menu.addAction(pinned ? QStringLiteral("取消置顶") : QStringLiteral("置顶"));
        QAction *deleteAction = menu.addAction(QStringLiteral("删除"));
        QAction *chosen = menu.exec(m_historyView->viewport()->mapToGlobal(pos));
        if (chosen == normalPasteAction) {
            onNormalPasteTriggered();
        } else if (chosen == plainTextPasteAction) {
            onPlainTextPasteTriggered();
        } else if (chosen == pinAction) {
            onPinSelectedTriggered();
        } else if (chosen == deleteAction) {
            onDeleteSelectedTriggered();
        }
    });

    layout->addLayout(searchLayout);
    layout->addWidget(m_historyView);

    setCentralWidget(central);
    statusBar();
}

QIcon MainWindow::buildFallbackIcon() const
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(60, 130, 220));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(4, 4, 56, 56, 12, 12);
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(34);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QStringLiteral("L"));
    painter.end();
    return QIcon(pixmap);
}

void MainWindow::setupTrayIcon()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    const QIcon icon = buildFallbackIcon();
    m_trayIcon = new QSystemTrayIcon(icon, this);
    m_trayIcon->setToolTip(QStringLiteral("LitePaste"));

    QMenu *menu = new QMenu(this);
    QAction *showHideAction = menu->addAction(QStringLiteral("显示/隐藏主窗口"));
    connect(showHideAction, &QAction::triggered, this, &MainWindow::toggleVisibility);

    m_togglePauseAction = menu->addAction(QStringLiteral("暂停记录"));
    m_togglePauseAction->setCheckable(true);
    connect(m_togglePauseAction, &QAction::triggered, this, &MainWindow::onTogglePauseTriggered);

    menu->addSeparator();
    QAction *quitAction = menu->addAction(QStringLiteral("退出"));
    connect(quitAction, &QAction::triggered, this, [this]() { emit quitRequested(); });

    m_trayIcon->setContextMenu(menu);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            toggleVisibility();
        }
    });

    m_trayIcon->show();
    setWindowIcon(icon);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        event->ignore();
        return;
    }
    event->accept();
}

void MainWindow::toggleVisibility()
{
    if (!isVisible()) {
        // 在自己获得焦点之前，让外部先记录当前系统前台窗口。
        emit windowAboutToActivate();
    }
    setVisible(!isVisible());
    if (isVisible()) {
        raise();
        activateWindow();
    }
}

void MainWindow::onSearchTextEdited(const QString & /*text*/)
{
    m_searchDebounceTimer->start(kSearchDebounceMs);
}

void MainWindow::onSearchDebounceTimeout()
{
    m_model->setFilter(m_searchEdit->text());
}

void MainWindow::onTogglePauseTriggered()
{
    m_paused = !m_paused;
    if (m_togglePauseAction) {
        m_togglePauseAction->setChecked(m_paused);
    }
    emit monitoringPausedChanged(m_paused);
}

void MainWindow::onPinSelectedTriggered()
{
    const QModelIndex index = m_historyView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    m_repository->togglePinned(m_model->idAt(index.row()));
}

void MainWindow::onDeleteSelectedTriggered()
{
    const QModelIndex index = m_historyView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    m_repository->removeItem(m_model->idAt(index.row()));
}

void MainWindow::onNormalPasteTriggered()
{
    const QModelIndex index = m_historyView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    requestPasteForRow(index.row(), PasteMode::Normal);
}

void MainWindow::onPlainTextPasteTriggered()
{
    const QModelIndex index = m_historyView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    requestPasteForRow(index.row(), PasteMode::PlainText);
}

void MainWindow::onItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    requestPasteForRow(index.row(), PasteMode::Normal);
}

void MainWindow::requestPasteForRow(int row, PasteMode mode)
{
    const QString text = m_model->data(m_model->index(row, 0), ClipboardHistoryModel::FullTextRole).toString();
    if (text.isEmpty()) {
        return;
    }
    emit pasteRequested(text, mode);
}

void MainWindow::onClearAllTriggered()
{
    if (QMessageBox::question(this, QStringLiteral("确认清空"), QStringLiteral("确定要清空全部历史记录吗？此操作不可撤销。"))
        != QMessageBox::Yes) {
        return;
    }
    m_repository->clear();
}

void MainWindow::onRepositoryRejected(const QString &reason)
{
    statusBar()->showMessage(reason, 3000);
}

void MainWindow::onPersistenceFailed(const QString &reason)
{
    if (m_trayIcon) {
        m_trayIcon->showMessage(QStringLiteral("LitePaste"), reason, QSystemTrayIcon::Warning, 4000);
    }
}

void MainWindow::onPasteFailed(const QString &reason)
{
    statusBar()->showMessage(reason, 4000);
}
