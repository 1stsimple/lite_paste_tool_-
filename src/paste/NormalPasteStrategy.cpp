#include "PasteStrategy.h"

namespace
{

// 普通粘贴：保留原始换行和 Unicode 文本内容，不做任何转换。
class NormalPasteStrategy : public PasteStrategy
{
public:
    QString transform(const QString &rawText) const override { return rawText; }
};

} // namespace

std::unique_ptr<PasteStrategy> createNormalPasteStrategy()
{
    return std::unique_ptr<PasteStrategy>(new NormalPasteStrategy());
}
