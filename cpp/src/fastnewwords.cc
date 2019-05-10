// Copyright (c) 2019-present, Tencent, Inc.
// All rights reserved.
// 
// Author: Haojie Pan
// Email: jasonhjpan@tencent.com
//

#include "fastnewwords.h"

#include <math.h>

#include <iomanip>
#include <iostream>
#include <numeric>
#include <limits>
#include <stack>
#include <map>

#include "myutils.h"

namespace fastnewwords {

FastNewWords::FastNewWords() {
    this->map_type = "hash";
    this->max_gram = 4;
    this->min_count = 5;
    this->min_solidity.push_back(1);
    for (size_t i = 1; i <= max_gram; ++i) {
        this->min_solidity.push_back(this->min_solidity[i - 1] * 5);
    }
    this->min_entropy = 2.0;
}

FastNewWords::FastNewWords(const std::string map_type, const size_t max_gram,
                           const size_t min_count, const float base_solidity,
                           const float min_entropy) {
    this->map_type = map_type;
    this->max_gram = max_gram;
    this->min_count = min_count;
    this->min_solidity.push_back(1);
    for (size_t i = 1; i <= max_gram; ++i) {
        this->min_solidity.push_back(
            this->min_solidity[i - 1] * base_solidity);
    }
    this->min_entropy = min_entropy;
}

void FastNewWords::insertWord(dict_t& d, const word_t& wd, 
                              const word_stat_t& ws) {
    d.insert({wd, ws});
}

void FastNewWords::insertWord(trie_t& d, const word_t& wd, 
                              const word_stat_t& ws) {
    d.insert(wd, ws);
}

word_t FastNewWords::getKey(dict_t::const_iterator& it) {
    return it->first;
}

word_t FastNewWords::getKey(trie_t::const_iterator& it) {
    return it.key();
}

word_stat_t FastNewWords::getValue(dict_t::const_iterator& it) {
    return it->second;
}

word_stat_t FastNewWords::getValue(trie_t::const_iterator& it) {
    return it.value();
}

count_t FastNewWords::getUnigramSum(const dict_t& dict) {
    count_t cnt = 0;
    for (auto& kv: dict) {
        cnt += kv.second.first;
    }
    return cnt;
}

count_t FastNewWords::getUnigramSum(const trie_t& dict) {
    count_t cnt = 0;
    for(auto it = dict.begin(); it != dict.end(); ++it) {
        cnt += it.value().first;
    }
    return cnt;
}

// Compute the solidity of a word, `Solidity` is a variant of PMI
// Example:
//      Solidity (ABC) = min {P(ABC) / P(A)P(BC), P(ABC) / P(AB)P(C)}
template<typename T>
float FastNewWords::solidity(const word_t& word, 
                             const T& d, 
                             const count_t total) {
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
        max_occur = std::max(
            max_occur, d.at(lt_word).first * d.at(rt_word).first);
    }
    return max_occur == 0 ? 0.0: 1.0 * d.at(word).first / max_occur * total;
}


// Compute the left or right entropy of a word
// Example:
//      {D: 10, E: 20}{ABC}{F: 20, G: 20}
//      left_entropy(ABC) = -(1/3)log(1/3) - (2/3)log(2/3)
//      right_entropy(ABC) = -(1/2)log(1/2) - (1/2)log(1/2)
float FastNewWords::adjEntropy(const word_t& wd,
                               const std::vector<position_t>& positions,
                               const std::string& utf8_content,
                               const std::string& reversed_utf8_content,
                               bool left) {
    // first extracting all neighbor words into a map;
    std::map<word_t, count_t> dist;
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

// load the whole stream and get candidate ngrams
// Count and positions of ngrams would be saved
// Low frequency ngram would be filtered
template<typename T>
T FastNewWords::getCandidateNgrams(std::istream& inp_stream) {
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

    T dict;
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
        if (myutils::is_chinese(first_token) && 
            !myutils::have_punk(first_token)) {
            word_t word(first_token);
            if (dict.find(word) == dict.end()) {
                word_stat_t ws({1, {ptr}});
                insertWord(dict, word, ws);
            } else {
                word_stat_t &ws = dict[word];
                ws.first++;
                ws.second.push_back(ptr);
            }
            for (int word_len = 1; word_len < this->max_gram; ++word_len) {
                word_t next_token = myutils::get_first_utf8(
                    this->utf8_content, ptr + word.length());
                if (!myutils::is_chinese(next_token) ||
                    myutils::have_punk(next_token)) {
                    break;
                }
                word += next_token;
                if (dict.find(word) == dict.end()) {
                    word_stat_t ws({1, {ptr}});
                    insertWord(dict, word, ws);
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

// Filtering candidate ngrams by solidity & entropy
template<typename T>
score_list_t FastNewWords::filteredDicts(const T& dict) {
    score_list_t score_list;
    count_t uni_cnt = getUnigramSum(dict);
    int steps = 0, total_steps = dict.size();
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        word_t wd = getKey(it);
        word_stat_t ws = getValue(it);
        steps += 1;
        if (steps % 5000 == 0) {
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
        float _sol = solidity(wd, dict, uni_cnt);
        if (_sol < this->min_solidity[wd_size])
            continue;
        float _rt_e = adjEntropy(wd,
                                 ws.second,
                                 this->utf8_content, 
                                 this->reversed_utf8_content,
                                 false);
        if (_rt_e < this->min_entropy)
            continue;
        float _lt_e = adjEntropy(wd,
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

void FastNewWords::retrieve(std::istream& inp_stream, std::ostream& outp_stream) {
    score_list_t scores;
    if (this->map_type == "hash") {
        dict_t dict = getCandidateNgrams<dict_t>(inp_stream);
        scores = filteredDicts(dict);
    } 
    else if (this->map_type == "trie") {
        trie_t dict = getCandidateNgrams<trie_t>(inp_stream);
        scores = filteredDicts(dict);
    }

    // output
    for (auto kv: scores) {
        std::cout << kv.first << " ";
        std::cout << kv.second.count << " ";
        std::cout << kv.second.solidity << " ";
        std::cout << kv.second.entropy << " " << std::endl;
    }
}

void FastNewWords::rerank(std::istream& inp_stream, 
                          std::ostream& outp_stream,
                          const std::string& dict_path,
                          const std::string& stopwords_path,
                          const int topk,
                          const bool withscores){
    word_set_t dictionary;
    if (!dict_path.empty()) {
        std::ifstream infile(dict_path);
        word_t wd;
        while (infile >> wd) dictionary.insert(wd);
    } 

    word_set_t stopwords;
    if (!stopwords_path.empty()) {
        std::ifstream infile(stopwords_path);
        word_t wd;
        while (infile >> wd) stopwords.insert(wd);
    }
    
    word_t wd_;
    count_t cnt_;
    float sol_;
    float ent_;
    score_dict_t word2scores;
    while (inp_stream >> wd_ >> cnt_ >> sol_ >> ent_) {
        if (stopwords.find(wd_) != stopwords.end()) continue;
        if (dictionary.find(wd_) != dictionary.end()) continue;
        if (word2scores.find(wd_) == word2scores.end()) {
            WordScore wsc(cnt_, sol_, ent_);
            word2scores.insert({wd_, {1, wsc}});
        } else {
            auto &cnt_wsc = word2scores[wd_];
            cnt_wsc.second.count = (cnt_wsc.second.count * cnt_wsc.first + cnt_) /
                                    (cnt_wsc.first + 1); // Will loss precision
            cnt_wsc.second.solidity = (cnt_wsc.second.solidity * cnt_wsc.first + sol_) /
                                      (cnt_wsc.first + 1);
            cnt_wsc.second.entropy = (cnt_wsc.second.entropy * cnt_wsc.first + ent_) /
                                     (cnt_wsc.first + 1);
            cnt_wsc.first++;
        }
    }

    score_list_t scores;
    // output
    for (auto& kv: word2scores) {
        auto &ws = kv.second.second;
        if (ws.count < this->min_count)
            continue;
        if (ws.solidity < this->min_solidity[1])
            continue;
        if (ws.entropy < this->min_entropy)
            continue;
        scores.push_back({kv.first, ws});
    }

    auto comp = [] (const std::pair<word_t, WordScore>& a, 
                    const std::pair<word_t, WordScore>& b) {
                        return a.second.count > b.second.count;
                };

    sort(scores.begin(), scores.end(), comp);

    int cnt = 0;
    // output
    for (auto kv: scores) {
        if (topk > 0 && cnt == topk)
            break;
        cnt++;
        if (withscores) {
            std::cout << kv.first << " ";
            std::cout << kv.second.count << " ";
            std::cout << kv.second.solidity << " ";
            std::cout << kv.second.entropy << " " << std::endl;
        } else {
            std::cout << kv.first << std::endl;
        }
    }
}

} // namespace fastnewwords