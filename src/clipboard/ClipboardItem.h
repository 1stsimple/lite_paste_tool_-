#pragma once

#include <QDateTime>
#include <QString>
#include <string>

struct ClipboardItem
{
    std::string id;
    QString text;
    QDateTime createdAt;
    bool pinned = false;
};
