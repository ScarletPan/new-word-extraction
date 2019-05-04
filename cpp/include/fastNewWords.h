#ifndef __infoEnt_h__
#define __infoEnt_h__

#include "myutils.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <math.h>
#include <numeric>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fastnewwords {

class WordStats;
class WordScore;

using count_t = size_t;
using adj_words_t = std::unordered_map<std::string, count_t>;
using dict_t = std::unordered_map<std::string, WordStats>;
using score_pair_t = std::pair<std::string, WordScore>;
using score_dict_t = std::unordered_map<std::string, WordScore>;
using score_vec_t = std::vector<score_pair_t>;
using pos_t = std::vector<std::pair<int, int>>;
using wn2positions_t = std::unordered_map<std::string, pos_t>;
using sentence_t = std::vector<std::vector<std::string>>;


class WordStats {
public:
    count_t count;
    adj_words_t left_adj_words;
    adj_words_t right_adj_words;
    WordStats() {}
    WordStats(const count_t cnt) {
        this->count = cnt;
    }
};

class WordScore {
public:
    count_t count;
    float solidity;
    float entropy;
    WordScore() {}
    WordScore(const count_t cnt, const float s, const float e) {
        this->count = cnt;
        this->solidity = s;
        this->entropy = e;
    };
};

class FastNewWords {
public:
    FastNewWords();
    FastNewWords(const size_t max_gram, const size_t min_count, const float base_solidity, const float min_entropy);

    score_vec_t discover(sentence_t& sentences);
    
private:
    size_t max_gram;
    size_t min_count;
    std::vector<float> min_solidity;
    float min_entropy;

    dict_t getCandidateNgrams(sentence_t& sentences);

    dict_t mergeNdicts(const std::vector<dict_t>& dict_list);

    std::vector<dict_t> splitNdicts(const dict_t& dict);

    score_vec_t filteredDicts(const dict_t& dict);

    count_t getUnigramSum(const dict_t& dict);

    float solidity(const std::string& word, const dict_t& d);

    float entropy(const adj_words_t& adj_words);
};

}


#endif