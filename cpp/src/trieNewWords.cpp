#include "trieNewWords.h"

namespace trienewwords {

TrieNewWords::TrieNewWords() {
    this->max_gram = 4;
    this->min_count = 5;
    this->min_solidity.push_back(1);
    for (size_t i = 1; i <= max_gram; ++i) {
        this->min_solidity.push_back(this->min_solidity[i - 1] * 5);
    }
    this->min_entropy = 2.0;
}


TrieNewWords::TrieNewWords(const size_t max_gram, const size_t min_count, const float base_solidity, const float min_entropy) {
    this->max_gram = max_gram;
    this->min_count = min_count;
    this->min_solidity.push_back(1);
    for (size_t i = 1; i <= max_gram; ++i) {
        this->min_solidity.push_back(this->min_solidity[i - 1] * base_solidity);
    }
    this->min_entropy = min_entropy;
}


dict_t TrieNewWords::getCandidateNgrams(std::istream& inp_stream) {
    std::string utf8_content = "";
    std::stack<word_t> stk;
    while (true) {
        word_t token = myutils::get_next_if_utf8(inp_stream);
        if (token.empty()) break;
        stk.push(token);
        utf8_content += token;
    }

    // Word Count
    dict_t dict;
    position_t ptr = 0;
    while (ptr < utf8_content.length()) {
        word_t first_token = myutils::get_first_utf8(utf8_content, ptr);
        if (myutils::is_chinese(first_token) && !myutils::have_punk(first_token)) {
            word_t word(first_token);
            if (dict.find(word) == dict.end()) {
                word_stat_t ws(1, adj_word_t());
                dict.insert(word, ws); // hat trie
                // dict.insert({word, ws}); // std::unordered_map
            } else {
                dict[word].first++;
            }
            dict[word].first++;
            for (int word_len = 1; word_len < this->max_gram; ++word_len) {
                word_t next_token = myutils::get_first_utf8(utf8_content, ptr + word.length());
                if (!myutils::is_chinese(next_token) || myutils::have_punk(next_token)) {
                    break;
                }
                word += next_token;
                if (dict.find(word) == dict.end()) {
                    word_stat_t ws(1, adj_word_t());
                    dict.insert(word, ws); // hat trie
                    // dict.insert({word, ws}); // std::unordered_map
                } else {
                    dict[word].first++;
                }
            }
        }
        ptr += first_token.length();        
    }

    // Add adj char for every valid word
    ptr = 0;
    word_t previous_token = "$";
    while (ptr < utf8_content.length()) {
        word_t first_token = myutils::get_first_utf8(utf8_content, ptr);
        if (dict.find(first_token) != dict.end()) {
            word_t word(first_token);
            if (dict[word].first >= this->min_count) {
                for (int word_len = 1; word_len < this->max_gram + 1; ++word_len) {
                    word_t next_token = myutils::get_first_utf8(utf8_content, ptr + word.length());
                    if (word_len > 1) {
                        word_stat_t& ws = dict[word];
                        // this is a hack, count(abc) < thr => count(abcd) < thr;
                        if (ws.first < this->min_count)
                            break;
                        if (ws.second.find(previous_token) == ws.second.end())
                            ws.second.insert({previous_token, {1, 0}});
                        else
                            ws.second[previous_token].first += 1;

                        if (!next_token.empty() || next_token == "\n") {
                            if (ws.second.find(next_token) == ws.second.end())
                                ws.second.insert({next_token, {0, 1}});
                            else
                                ws.second[next_token].second += 1;
                        }
                    }
                    word += next_token;
                    // abc not in dict => abcd not in dict
                    if (dict.find(word) == dict.end())
                        break;
                }
            }
        }
        ptr += first_token.length();
        previous_token = first_token;
    }
    return dict;
}


score_list_t TrieNewWords::filteredDicts(const dict_t& dict) {
    score_list_t score_list;

    count_t uni_cnt = getUnigramSum(dict);
    int steps = 0, total_steps = dict.size();
    // for (auto& kv: dict) {
    //     word_t wd = kv.first;
    //     word_stat_t ws = kv.second;
    for(auto it = dict.begin(); it != dict.end(); ++it) {
        steps += 1;
        if (steps % 100 == 0) {
            std::cerr << std::fixed;
            std::cerr << "Filterring: "
                      << std::setprecision(1) << std::setw(5)
                      << 1.0 * steps / total_steps * 100 << "%" << " \r";
            std::cerr << std::flush;
        }
        word_stat_t ws = it.value();
        if (ws.first < this->min_count)
            continue;
        word_t wd = it.key();
        size_t wd_size = myutils::size_of_utf8(wd);
        if (wd_size < 2)
            continue;
        float _sol = solidity(wd, dict) * uni_cnt;
        if (_sol < this->min_solidity[wd_size])
            continue;
        float adj_ent = adjEntropy(ws.second);
        if (adj_ent < this->min_entropy)
            continue;
        
        WordScore wsc(ws.first, _sol, adj_ent);
        score_list.push_back({wd, wsc});
    }
    std::cerr << "Filterred by solidity & entropy: 100%" << std::endl;
    return score_list;
}


count_t TrieNewWords::getUnigramSum(const dict_t& dict) {
    count_t cnt = 0;
    // for (auto& kv: dict) {
    //     cnt += kv.second.first;
    // }
    for(auto it = dict.begin(); it != dict.end(); ++it) {
        cnt += it.value().first;
    }
    return cnt;
}


float TrieNewWords::solidity(const std::string& word, const dict_t& d) {
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


float TrieNewWords::entropy(const std::vector<count_t>& counts) {
    float _ent = 0;
    count_t total = 0;
    for (auto cnt: counts)
        total += cnt;
    for (auto cnt: counts) {
        float pk = 1.0 * cnt / total;
        _ent += -std::log(pk) * pk; 
    }
    return _ent;
}


float TrieNewWords::adjEntropy(const adj_word_t& adj_words) {
    // first extracting all neighbor words;
    std::vector<count_t> left_counts;
    left_counts.reserve(adj_words.size());
    std::vector<count_t> right_counts;
    right_counts.reserve(adj_words.size());
    for (auto& kv: adj_words) {
        left_counts.push_back(kv.second.first);
        right_counts.push_back(kv.second.second);
    }

    return std::min(entropy(left_counts),
                    entropy(right_counts));
}


score_list_t TrieNewWords::discover(std::istream& inp_stream) {
    dict_t dict = getCandidateNgrams(inp_stream);
    score_list_t score_vec = filteredDicts(dict);
    return score_vec;
}

}
