#include "AppConfig.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>

namespace
{
const int kDefaultMaxHistoryCount = 1000;
const int kDefaultMaxItemTextLength = 200 * 1024; // 200K 字符，避免超大文本拖垮界面
const int kDefaultRetentionDays = 0;
}

AppConfig::AppConfig()
    : m_settings(new QSettings(QSettings::IniFormat, QSettings::UserScope, "LitePaste", "LitePaste"))
{
}

AppConfig::~AppConfig()
{
    delete m_settings;
}

int AppConfig::maxHistoryCount() const
{
    return m_settings->value("history/maxCount", kDefaultMaxHistoryCount).toInt();
}

void AppConfig::setMaxHistoryCount(int count)
{
    m_settings->setValue("history/maxCount", count);
}

int AppConfig::maxItemTextLength() const
{
    return m_settings->value("history/maxItemTextLength", kDefaultMaxItemTextLength).toInt();
}

void AppConfig::setMaxItemTextLength(int length)
{
    m_settings->setValue("history/maxItemTextLength", length);
}

int AppConfig::retentionDays() const
{
    return m_settings->value("history/retentionDays", kDefaultRetentionDays).toInt();
}

void AppConfig::setRetentionDays(int days)
{
    m_settings->setValue("history/retentionDays", days);
}

bool AppConfig::restoreClipboardAfterPaste() const
{
    return m_settings->value("paste/restoreClipboardAfterPaste", true).toBool();
}

void AppConfig::setRestoreClipboardAfterPaste(bool enabled)
{
    m_settings->setValue("paste/restoreClipboardAfterPaste", enabled);
}

QString AppConfig::historyFilePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/history.json";
}
