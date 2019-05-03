#include "infoEnt.h"

namespace fastnewwords {

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


FastNewWords::FastNewWords() {
    this->max_gram = 4;
    this->min_count = 5;
    this->min_solidity.push_back(1);
    for (size_t i = 1; i <= max_gram; ++i) {
        this->min_solidity.push_back(this->min_solidity[i - 1] * 5);
    }
    this->min_entropy = 2.0;
}


FastNewWords::FastNewWords(const size_t max_gram, const size_t min_count, const float base_solidity, const float min_entropy) {
    this->max_gram = max_gram;
    this->min_count = min_count;
    this->min_solidity.push_back(1);
    for (size_t i = 1; i <= max_gram; ++i) {
        this->min_solidity.push_back(this->min_solidity[i - 1] * base_solidity);
    }
    this->min_entropy = min_entropy;
}


dict_t FastNewWords::getCandidateNgrams(sentence_t& sentences) {
    wn2positions_t wn2positions;
    for (size_t i = 0; i < sentences.size(); ++i) {
        for (size_t j = 0; j < sentences[i].length(); ++j) {
            for (size_t word_len = 1; word_len <= this->max_gram; ++word_len) {
                if (j + word_len > sentences[i].length())
                    continue;
                auto word = sentences[i].substr(j, word_len);
                if (wn2positions.find(word) == wn2positions.end()) {
                    wn2positions[word] = pos_t();
                }
                wn2positions[word].push_back(std::make_pair<int, int>(i, j));
            }
        }
    }

    dict_t dict;
    wn2positions_t::iterator it;
    for (it = wn2positions.begin(); it != wn2positions.end(); it++) {
        if (it->second.size() < this->min_count)
            continue;
        if (!is_chinese(it->first))
            continue;
        if (dict.find(it->first) == dict.end()) {
            dict.insert(std::make_pair(it->first, WordStats(it->second.size())));
        }
        for (auto pos: it->second) {
            int i = pos.first, j = pos.second;
            if (j > 0) {
                wchar_t c = sentences[i][j - 1];
                if (dict[it->first].left_adj_words.find(c) == dict[it->first].left_adj_words.end())
                    dict[it->first].left_adj_words[c] = 0;
                dict[it->first].left_adj_words[c]++;
            }
            if (j + it->first.size() < sentences[i].length()) {
                wchar_t c = sentences[i][j + it->first.size()];
                if (dict[it->first].right_adj_words.find(c) == dict[it->first].right_adj_words.end())
                    dict[it->first].right_adj_words[c] = 0;
                dict[it->first].right_adj_words[c]++;
            }
        }
    }

    return dict;
}

score_vec_t FastNewWords::filteredDicts(dict_t& dict) {
    score_dict_t score_dict;
    count_t uni_cnt = getUnigramSum(dict);
    for (auto& kv: dict) {
        if (kv.first.length() < 2)
            continue;
        if (!is_chinese(kv.first))
            continue;
        float _sol = solidity(kv.first, dict) * uni_cnt;
        if (_sol < this->min_solidity[kv.first.length()])
            continue;
        float _lt_e = entropy(kv.second.left_adj_words);
        if (_lt_e < this->min_entropy)
            continue;
        float _rt_e = entropy(kv.second.right_adj_words);
        if (_rt_e < this->min_entropy)
            continue;
        WordScore ws(kv.second.count, _sol, std::min(_lt_e, _rt_e));
        score_dict.insert(std::make_pair(kv.first, ws));
    }
    return score_vec_t(score_dict.begin(), score_dict.end());
}


count_t FastNewWords::getUnigramSum(dict_t& dict) {
    count_t cnt = 0;
    for (auto& kv: dict) {
        cnt += kv.second.count;
    }
    return cnt;
}


float FastNewWords::solidity(const std::wstring& word, const dict_t& d) {
    if (d.find(word) == d.end())
        return 0;
    count_t max_occur = 0;
    for (size_t i = 1; i < word.length(); ++i) {
        std::wstring lt_word = word.substr(0, i);
        if (d.find(lt_word) == d.end())
            continue;
        std::wstring rt_word = word.substr(i, word.length() - i); 
        if (d.find(rt_word) == d.end())
            continue;
        max_occur = std::max(max_occur, d.at(lt_word).count * d.at(rt_word).count);
    }
    return max_occur == 0 ? 0.0: 1.0 * d.at(word).count / max_occur;
}


float FastNewWords::entropy(adj_words_t& adj_words) {
    float _ent = 0;
    count_t total = 0;
    for (auto& kv: adj_words)
        total += kv.second;
    for (auto& kv: adj_words) {
        float pk = 1.0 * kv.second / total;
        _ent += -std::log(pk) * pk; 
    }
    return _ent;
    
}


score_vec_t FastNewWords::discover(sentence_t& sentences) {
    dict_t dict = getCandidateNgrams(sentences);
    score_vec_t score_vec = filteredDicts(dict);
    return score_vec;
}

}
