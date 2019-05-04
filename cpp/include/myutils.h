#ifndef __UTILS_H__
#define __UTILS_H__
#include <codecvt>
#include <string>
#include <locale>
#include <unordered_set>

namespace myutils {

static const std::unordered_set<wchar_t> PUNKS = {
    L'！', L'？', L'｡', L'。', L'＂', L'＃', L'＄', L'％', L'＆', L'＇', L'（', L'）', L'＊', L'＋', L'，', L'－', L'／', L'：', L'；', L'＜', L'＝', L'＞', L'＠', L'［', L'＼', L'］', L'＾', L'＿', L'｀', L'｛', L'｜', L'｝', L'～', L'｟', L'｠', L'｢', L'｣', L'､', L'、', L'〃', L'》', L'「', L'」', L'『', L'』', L'【', L'】', L'〔', L'〕', L'〖', L'〗', L'〘', L'〙', L'〚', L'〛', L'〜', L'〝', L'〞', L'〟', L'〰', L'〾', L'〿', L'–', L'—', L'‘', L'’', L'‛', L'“', L'”', L'„', L'‟', L'…', L'‧', L'﹏', L'.'
};

using convert_type = std::codecvt_utf8<wchar_t>;

bool is_chinese(const std::wstring& s);

bool have_punk(const std::wstring& s);

}

#endif