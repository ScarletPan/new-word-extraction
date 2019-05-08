#include "fastNewWords.h"

namespace fastnewwords {

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


dict_t FastNewWords::getCandidateNgrams(std::istream& inp_stream) {
    dict_t dict;
    std::stack<word_t> stk;
    std::cerr << "Load whole documents...";
    while (true) {
        word_t token = myutils::get_next_if_utf8(inp_stream);
        if (token.empty()) break;
        stk.push(token);
        this->utf8_content += token;
    }
    while (!stk.empty()) {
        this->reversed_utf8_content += stk.top();
        stk.pop();
    }
    std::cerr << " Done."  << std::endl;

    position_t ptr = 0;
    size_t content_len = this->utf8_content.length();
    while (ptr < this->utf8_content.length()) {
        if (ptr % 50000 == 0) {
            std::cerr << std::fixed;
            std::cerr << "Word counting: " 
                    << std::setprecision(1) << std::setw(5) 
                    << 1.0 * ptr / content_len * 100 << "%" << " \r";
            std::cerr << std::flush;
        }
        word_t first_token = myutils::get_first_utf8(this->utf8_content, ptr);
        if (myutils::is_chinese(first_token) && !myutils::have_punk(first_token)) {
            word_t word(first_token);
            if (dict.find(word) == dict.end()) {
                word_stat_t ws({1, {ptr}});
                dict.insert({word, ws});
            } else {
                word_stat_t &ws = dict[word];
                ws.first++;
                ws.second.push_back(ptr);
            }
            for (int word_len = 1; word_len < this->max_gram; ++word_len) {
                word_t next_token = myutils::get_first_utf8(this->utf8_content, ptr + word.length());
                if (!myutils::is_chinese(next_token) || myutils::have_punk(next_token)) {
                    break;
                }
                word += next_token;
                if (dict.find(word) == dict.end()) {
                    word_stat_t ws({1, {ptr}});
                    dict.insert({word, ws});
                } else {
                    word_stat_t &ws = dict[word];
                    ws.first++;
                    ws.second.push_back(ptr);
                }
            }
        }
        ptr += first_token.length();        
    }
    std::cerr << "Word counting: " 
              << std::setprecision(1) << std::setw(5) 
              << 100 << "%" << std::endl;

    return dict;
}


score_list_t FastNewWords::filteredDicts(const dict_t& dict) {
    score_list_t score_list;

    count_t uni_cnt = getUnigramSum(dict);
    int steps = 0, total_steps = dict.size();
    for (auto& kv: dict) {
        word_t wd = kv.first;
        word_stat_t ws = kv.second;
        steps += 1;
        if (steps % 100 == 0) {
            std::cerr << std::fixed;
            std::cerr << "Filterring: "
                      << std::setprecision(1) << std::setw(5)
                      << 1.0 * steps / total_steps * 100 << "%" << " \r";
            std::cerr << std::flush;
        }
        if (ws.first < this->min_count)
            continue;
        size_t wd_size = myutils::size_of_utf8(wd);
        if (wd_size < 2)
            continue;
        float _sol = solidity(wd, dict) * uni_cnt;
        if (_sol < this->min_solidity[wd_size])
            continue;
        float _rt_e = entropy(wd,
                              ws.second,
                              this->utf8_content, 
                              this->reversed_utf8_content,
                              false);
        if (_rt_e < this->min_entropy)
            continue;
        float _lt_e = entropy(wd,
                              ws.second,
                              this->utf8_content, 
                              this->reversed_utf8_content,
                              true);
        if (_lt_e < this->min_entropy)
            continue;
        WordScore wsc(ws.first, _sol, std::min(_lt_e, _rt_e));
        score_list.push_back({wd, wsc});
    }
    std::cerr << "Filterred by solidity & entropy: 100%" << std::endl;
    return score_list;
}


count_t FastNewWords::getUnigramSum(const dict_t& dict) {
    count_t cnt = 0;
    for (auto& kv: dict) {
        cnt += kv.second.first;
    }
    return cnt;
}


float FastNewWords::solidity(const std::string& word, const dict_t& d) {
    if (d.find(word) == d.end())
        return 0;
    count_t max_occur = 0;
    std::vector<std::string> tokens = myutils::split_utf_str(word);
    if (tokens.size() == 1)
        return 0.0;
    for (size_t i = 1; i < tokens.size(); ++i) {
        std::string lt_word = std::accumulate(
                tokens.begin(), tokens.begin() + i, std::string(""));
        if (d.find(lt_word) == d.end())
            continue;
        std::string rt_word = std::accumulate(
                tokens.begin() + i, tokens.end(), std::string(""));
        if (d.find(rt_word) == d.end())
            continue;
        max_occur = std::max(max_occur, d.at(lt_word).first * d.at(rt_word).first);
    }
    return max_occur == 0 ? 0.0: 1.0 * d.at(word).first / max_occur;
}


float FastNewWords::entropy(const word_t& wd,
                            const std::vector<position_t>& positions,
                            const std::string& utf8_content,
                            const std::string& reversed_utf8_content,
                            bool left) {
    // first extracting all neighbor words;
    std::unordered_map<word_t, count_t> dist;
    size_t content_len = utf8_content.size();
    for (auto pos: positions) {
        word_t neighbor_wd;
        if (left) {
            neighbor_wd = myutils::get_first_utf8(
                    reversed_utf8_content, content_len - pos);
        } else {
            neighbor_wd = myutils::get_first_utf8(
                    utf8_content, pos + wd.length());
        }
        if (!neighbor_wd.empty())
            dist[neighbor_wd]++;
    }

    // compute entropy
    float _ent = 0;
    count_t total = 0;
    for (auto& kv: dist)
        total += kv.second;
    for (auto& kv: dist) {
        float pk = 1.0 * kv.second / total;
        _ent += -std::log(pk) * pk; 
    }
    return _ent;
    
}


score_list_t FastNewWords::discover(std::istream& inp_stream) {
    dict_t dict = getCandidateNgrams(inp_stream);
    score_list_t score_vec = filteredDicts(dict);
    return score_vec;
}

}