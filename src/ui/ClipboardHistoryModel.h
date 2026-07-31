#pragma once

#include <vector>

#include <QAbstractListModel>

#include "clipboard/ClipboardItem.h"

class ClipboardRepository;

// 将 ClipboardRepository 的数据以 Model/View 方式暴露给 QListView，
// 避免为每条历史记录创建独立 QWidget。
// 输入：ClipboardRepository 的 itemsReset 信号、用户设置的搜索关键字。
// 输出：标准 QAbstractListModel 接口供视图消费。
class ClipboardHistoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role
    {
        IdRole = Qt::UserRole + 1,
        FullTextRole,
        PinnedRole,
        CreatedAtRole,
    };

    explicit ClipboardHistoryModel(ClipboardRepository *repository, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    std::string idAt(int row) const;

public slots:
    void setFilter(const QString &keyword);

private slots:
    void onRepositoryItemsReset();

private:
    void refreshSnapshot();

    ClipboardRepository *m_repository;
    QString m_filter;
    std::vector<ClipboardItem> m_snapshot;
};
