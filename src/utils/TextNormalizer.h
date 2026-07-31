#pragma once

#include <QString>

// 纯文本清理工具：统一换行符，并可选去除行尾空白、连续空行和不可见控制字符。
// 所有函数均为无状态纯函数，输入输出都是 QString，便于单元测试。
namespace TextNormalizer
{

struct Options
{
    bool trimTrailingWhitespace = true;
    bool collapseBlankLines = true;
    bool stripControlCharacters = true;
};

// 将 \r\n 和单独的 \r 统一替换为 \n。空文本原样返回。
QString unifyLineEndings(const QString &text);

// 依次执行：统一换行符 -> 去控制字符 -> 去行尾空白 -> 合并连续空行。
// 每一步均可通过 options 单独关闭。
QString normalize(const QString &text, const Options &options = Options());

} // namespace TextNormalizer
