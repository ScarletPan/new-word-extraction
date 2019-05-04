#include "myutils.h"

namespace myutils
{

bool is_chinese(const std::wstring& s) {
    std::wstring_convert<convert_type, wchar_t> converter;
    std::string str = converter.to_bytes(s);
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


bool have_punk(const std::wstring& s) {
    for (wchar_t wch: s) {
        if (PUNKS.find(wch) != PUNKS.end()) {
            return true;
        }
    }
    return false;
}

    
} // namespace utils