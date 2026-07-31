#pragma once

#include <vector>

#include <QString>

#include "clipboard/ClipboardItem.h"

// 历史记录持久化接口。实现必须是无状态或线程安全的，
// 因为 save() 可能从非 UI 线程调用以避免阻塞界面。
class HistoryStorage
{
public:
    virtual ~HistoryStorage() = default;

    // 从持久化介质读取全部历史记录，按存储顺序填入 items。
    // 失败时返回 false 并在 errorMessage 中写入原因（若非空）；不会抛出异常。
    virtual bool load(std::vector<ClipboardItem> &items, QString *errorMessage = nullptr) = 0;

    // 将 items 完整写入持久化介质（覆盖式写入）。
    // 失败时返回 false 并在 errorMessage 中写入原因（若非空）。
    virtual bool save(const std::vector<ClipboardItem> &items, QString *errorMessage = nullptr) = 0;
};
