#include "myutils.h"

#include <locale>

namespace myutils
{

std::string wstringToString(const std::wstring str) {
    unsigned len = str.size() * 4;
    setlocale(LC_CTYPE, "");
    char *p = new char[len];
    wcstombs(p,str.c_str(),len);
    std::string str1(p);
    delete[] p;
    return str1;
}

bool is_chinese(const std::string& str) {
    unsigned char utf[4] = {0};
    unsigned char unicode[3] = {0};
    bool res = false;
    for (int i = 0; i < str.length(); i++) {
        if ((str[i] & 0x80) == 0) {
            res = false;
        }
        else{
            utf[0] = str[i];
            utf[1] = str[i + 1];
            utf[2] = str[i + 2];
            i++;
            i++;
            unicode[0] = ((utf[0] & 0x0F) << 4) | ((utf[1] & 0x3C) >>2);
            unicode[1] = ((utf[1] & 0x03) << 6) | (utf[2] & 0x3F);

            if(unicode[0] >= 0x4e && unicode[0] <= 0x9f) {
                if (unicode[0] == 0x9f && unicode[1] > 0xa5)
                    res = false;
                else
                    res = true;
            } else
                res = false;
        }
    }
    return res;
}

bool is_chinese(const std::wstring& s) {
    std::string str = wstringToString(s);
    return is_chinese(str);
}

bool have_punk(const std::wstring& s) {
    for (wchar_t wch: s) {
        if (PUNKS.find(wch) != PUNKS.end()) {
            return true;
        }
    }
    return false;
}

bool have_punk(const std::string& s) {
    int i = 0;
    std::string tmp;
    while (i < s.length()) {
        tmp = get_first_utf8(s, i);
        if (PUNKS_U8.find(tmp) != PUNKS_U8.end()) {
            return true;
        }
        if (tmp.empty()) break;
        i += tmp.length();
    }
    return false;
}

size_t get_utf8_len(const char ch) {
    size_t len =
            (ch >> 7) == 0 ? 1 : // 0xxxxxxx
            (~ch & 0x20) ? 2 :   // 110xxxxx 10xxxxxx
            (~ch & 0x10) ? 3: 4;
    return len;
}

std::string get_first_utf8(const std::string& utf8_str, const unsigned int st) {
    if (utf8_str.size() - st <= 0) return "";
    size_t len = get_utf8_len(utf8_str[st]);
    if (len > utf8_str.length() - st) {
        std::cerr << "UTF-8 decoding error " << std::endl;
        return "";
    }
    return utf8_str.substr(st, len);
}

std::string get_second_utf8(const std::string& utf8_str, const unsigned int st) {
    if (utf8_str.size() - st <= 0) return "";
    size_t cur_len = get_utf8_len(utf8_str[st]);
    return get_first_utf8(utf8_str, st + cur_len);
}

std::vector<std::string> split_utf_str(const std::string& utf8_str) {
    std::vector<std::string> res;
    int i = 0;
    while (i < utf8_str.length()) {
        std::string s = get_first_utf8(utf8_str, i);
        if (s.empty()) break;
        res.push_back(s);
        i += s.length();
    }
    return res;
}

std::string get_next_if_utf8(std::istream& utf8_if) {
    char ch;
    if (!utf8_if.get(ch)) {
        return "";
    }
    size_t len = get_utf8_len(ch);
    std::string str;
    str.resize(len, '\0');
    str[0] = ch;
    for (int i = 1; i < len; ++i) {
        if (!utf8_if.get(ch)) {
            return "";
        } else {
            str[i] = ch;
        }
    }
    return str;
}

size_t size_of_utf8(const std::string& utf8_str) {
    size_t cnt = 0;
    int i = 0;
    while (i < utf8_str.length()) {
        size_t len = get_utf8_len(utf8_str[i]);
        i += len;
        cnt++;
    }
    return cnt;
}

size_t size_of_unicode_cn(const std::string& unicode_str) {
    return unicode_str.size() / 2;
}

std::string utf8_to_unicode_cn(const std::string& utf8_str) {
    std::string buffer;
    size_t ptr = 0;
    while (ptr < utf8_str.size()) {
        size_t len = get_utf8_len(utf8_str[ptr]);
        if (len != 3 || (ptr + len) > utf8_str.size())
            return "";
        unsigned char first_byte = (utf8_str[ptr] & 0x0F) << 4;  // 1110 xxxx
        first_byte |= (utf8_str[ptr + 1] & 0x3C) >> 2; //10xx xxxx
        unsigned char second_byte = (utf8_str[ptr + 1] & 0x03) << 6;
        second_byte |= (utf8_str[ptr + 2] & 0x3F);
        buffer += first_byte;
        buffer += second_byte;
        ptr += len;
    }
    return buffer;
}

std::string unicode_to_utf8_cn(const std::string& unicode_str) {
    std::string buffer;
    for (int i = 0; i < unicode_str.size() / 2; ++i) {
        uint8_t first_byte = unicode_str[i * 2];
        uint8_t second_byte = unicode_str[i * 2 + 1];
        buffer += (char) (0xE0 | (first_byte >> 4));
        buffer += (char) (0x80 | ((first_byte << 2) & 0x3F) | (second_byte >> 6));
        buffer += (char) (0x80 | (second_byte & 0x3F));
    }
    return buffer;
}

} // namespace myutls