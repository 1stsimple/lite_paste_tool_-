#include "PasteStrategy.h"

#include "utils/TextNormalizer.h"

namespace
{

// 纯文本粘贴：统一换行符，并清理行尾空格、连续空行和不可见控制字符。
class PlainTextPasteStrategy : public PasteStrategy
{
public:
    QString transform(const QString &rawText) const override { return TextNormalizer::normalize(rawText); }
};

} // namespace

std::unique_ptr<PasteStrategy> createPlainTextPasteStrategy()
{
    return std::unique_ptr<PasteStrategy>(new PlainTextPasteStrategy());
}
