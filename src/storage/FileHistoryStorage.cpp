#include "FileHistoryStorage.h"

#include <QFile>
#include <QSaveFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QDir>

FileHistoryStorage::FileHistoryStorage(const QString &filePath)
    : m_filePath(filePath)
{
}

bool FileHistoryStorage::load(std::vector<ClipboardItem> &items, QString *errorMessage)
{
    items.clear();

    QFile file(m_filePath);
    if (!file.exists()) {
        // 首次启动、尚无历史文件不算错误。
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法打开历史文件: %1").arg(file.errorString());
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("历史文件解析失败: %1").arg(parseError.errorString());
        }
        return false;
    }

    const QJsonArray array = doc.array();
    items.reserve(static_cast<size_t>(array.size()));
    for (const QJsonValue &value : array) {
        const QJsonObject obj = value.toObject();
        ClipboardItem item;
        item.id = obj.value(QStringLiteral("id")).toString().toStdString();
        item.text = obj.value(QStringLiteral("text")).toString();
        item.createdAt = QDateTime::fromString(obj.value(QStringLiteral("createdAt")).toString(), Qt::ISODateWithMs);
        item.pinned = obj.value(QStringLiteral("pinned")).toBool(false);
        items.push_back(item);
    }

    return true;
}

bool FileHistoryStorage::save(const std::vector<ClipboardItem> &items, QString *errorMessage)
{
    QJsonArray array;
    for (const ClipboardItem &item : items) {
        QJsonObject obj;
        obj.insert(QStringLiteral("id"), QString::fromStdString(item.id));
        obj.insert(QStringLiteral("text"), item.text);
        obj.insert(QStringLiteral("createdAt"), item.createdAt.toString(Qt::ISODateWithMs));
        obj.insert(QStringLiteral("pinned"), item.pinned);
        array.append(obj);
    }

    QDir().mkpath(QFileInfo(m_filePath).absolutePath());

    // QSaveFile 保证写入过程中若崩溃或断电，不会破坏原有历史文件。
    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("无法写入历史文件: %1").arg(file.errorString());
        }
        return false;
    }

    file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));

    if (!file.commit()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("提交历史文件失败: %1").arg(file.errorString());
        }
        return false;
    }

    return true;
}
