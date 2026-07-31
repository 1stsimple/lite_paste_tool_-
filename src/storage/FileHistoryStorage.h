#pragma once

#include "HistoryStorage.h"

// 基于 JSON 文件的历史记录持久化实现。
// 每个实例只绑定一个文件路径；load/save 均为同步阻塞调用，
// 调用方（ClipboardRepository）负责将 save 派发到非 UI 线程执行。
class FileHistoryStorage : public HistoryStorage
{
public:
    explicit FileHistoryStorage(const QString &filePath);

    bool load(std::vector<ClipboardItem> &items, QString *errorMessage = nullptr) override;
    bool save(const std::vector<ClipboardItem> &items, QString *errorMessage = nullptr) override;

private:
    QString m_filePath;
};
