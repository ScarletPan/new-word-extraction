#ifndef __MYUTILS_H__
#define __MYUTILS_H__
#include <codecvt>
#include <fstream>
#include <iostream>
#include <string>
#include <locale>
#include <unordered_set>
#include <vector>

namespace myutils {

static const std::unordered_set<wchar_t> PUNKS = {
    L'！', L'？', L'｡', L'。', L'＂', L'＃', L'＄', L'％', L'＆', L'＇', L'（', L'）', L'＊', L'＋', L'，', L'－', L'／', L'：', L'；', L'＜', L'＝', L'＞', L'＠', L'［', L'＼', L'］', L'＾', L'＿', L'｀', L'｛', L'｜', L'｝', L'～', L'｟', L'｠', L'｢', L'｣', L'､', L'、', L'〃', L'》', L'「', L'」', L'『', L'』', L'【', L'】', L'〔', L'〕', L'〖', L'〗', L'〘', L'〙', L'〚', L'〛', L'〜', L'〝', L'〞', L'〟', L'〰', L'〾', L'〿', L'–', L'—', L'‘', L'’', L'‛', L'“', L'”', L'„', L'‟', L'…', L'‧', L'﹏', L'.'
};

static const std::unordered_set<std::string> PUNKS_U8 = {
    u8"！", u8"？", u8"｡", u8"。", u8"＂", u8"＃", u8"＄", u8"％", u8"＆", u8"＇", u8"（", u8"）", u8"＊", u8"＋", u8"，", u8"－", u8"／", u8"：", u8"；", u8"＜", u8"＝", u8"＞", u8"＠", u8"［", u8"＼", u8"］", u8"＾", u8"＿", u8"｀", u8"｛", u8"｜", u8"｝", u8"～", u8"｟", u8"｠", u8"｢", u8"｣", u8"､", u8"、", u8"〃", u8"》", u8"「", u8"」", u8"『", u8"』", u8"【", u8"】", u8"〔", u8"〕", u8"〖", u8"〗", u8"〘", u8"〙", u8"〚", u8"〛", u8"〜", u8"〝", u8"〞", u8"〟", u8"〰", u8"〾", u8"〿", u8"–", u8"—", u8"‘", u8"’", u8"‛", u8"“", u8"”", u8"„", u8"‟", u8"…", u8"‧", u8"﹏", u8"."
};

using convert_type = std::codecvt_utf8<wchar_t>;

bool is_chinese(const std::wstring& s);

bool is_chinese(const std::string& s);

bool have_punk(const std::wstring& s);

bool have_punk(const std::string& s);


size_t get_utf8_len(const char ch);
/*
    Get first utf8 character of a string
*/
std::string get_first_utf8(const std::string& utf8_str, const unsigned int st=0);


std::string get_second_utf8(const std::string& utf8_str, const unsigned int st=0);

/*
    Get Next utf8 character from input stream
*/
std::string get_next_if_utf8(std::istream& utf8_if);


std::vector<std::string> split_utf_str(const std::string& utf8_str);


size_t size_of_utf8(const std::string& utf8_str);

}

#endif