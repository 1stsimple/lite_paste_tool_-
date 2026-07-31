#pragma once

#include <memory>

#include <QString>

enum class PasteMode
{
    Normal,
    PlainText,
    WholeLine,
    Rectangle,
    Sequential
};

// 粘贴模式的统一接口：只负责把原始历史文本转换成该模式下
// 应该写入系统剪贴板的最终文本，不涉及按键模拟。
class PasteStrategy
{
public:
    virtual ~PasteStrategy() = default;
    virtual QString transform(const QString &rawText) const = 0;
};

std::unique_ptr<PasteStrategy> createNormalPasteStrategy();
std::unique_ptr<PasteStrategy> createPlainTextPasteStrategy();
