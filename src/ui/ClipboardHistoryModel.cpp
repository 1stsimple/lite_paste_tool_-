#include "ClipboardHistoryModel.h"

#include "clipboard/ClipboardRepository.h"

namespace
{
const int kPreviewMaxLength = 200;

QString buildPreview(const QString &text)
{
    int newlineIndex = text.indexOf('\n');
    QString firstLine = (newlineIndex >= 0) ? text.left(newlineIndex) : text;
    bool multiline = newlineIndex >= 0;

    if (firstLine.length() > kPreviewMaxLength) {
        firstLine = firstLine.left(kPreviewMaxLength) + QStringLiteral("…");
    } else if (multiline) {
        firstLine += QStringLiteral(" …");
    }
    return firstLine;
}
}

ClipboardHistoryModel::ClipboardHistoryModel(ClipboardRepository *repository, QObject *parent)
    : QAbstractListModel(parent)
    , m_repository(repository)
{
    connect(m_repository, &ClipboardRepository::itemsReset, this, &ClipboardHistoryModel::onRepositoryItemsReset);
    refreshSnapshot();
}

int ClipboardHistoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_snapshot.size());
}

QVariant ClipboardHistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_snapshot.size())) {
        return QVariant();
    }

    const ClipboardItem &item = m_snapshot[static_cast<size_t>(index.row())];

    switch (role) {
    case Qt::DisplayRole:
        return buildPreview(item.text);
    case Qt::ToolTipRole:
        return item.text;
    case IdRole:
        return QString::fromStdString(item.id);
    case FullTextRole:
        return item.text;
    case PinnedRole:
        return item.pinned;
    case CreatedAtRole:
        return item.createdAt;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ClipboardHistoryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[FullTextRole] = "fullText";
    roles[PinnedRole] = "pinned";
    roles[CreatedAtRole] = "createdAt";
    return roles;
}

std::string ClipboardHistoryModel::idAt(int row) const
{
    if (row < 0 || row >= static_cast<int>(m_snapshot.size())) {
        return std::string();
    }
    return m_snapshot[static_cast<size_t>(row)].id;
}

void ClipboardHistoryModel::setFilter(const QString &keyword)
{
    if (m_filter == keyword) {
        return;
    }
    m_filter = keyword;
    onRepositoryItemsReset();
}

void ClipboardHistoryModel::onRepositoryItemsReset()
{
    beginResetModel();
    refreshSnapshot();
    endResetModel();
}

void ClipboardHistoryModel::refreshSnapshot()
{
    m_snapshot = m_filter.isEmpty() ? m_repository->items() : m_repository->search(m_filter);
}
