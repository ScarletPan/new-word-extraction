#ifndef __infoEnt_h__
#define __infoEnt_h__

#include "myutils.h"
#include <tsl/htrie_map.h>
// #include <tsl/array-hash/array_map.h>
// #include <parallel_hashmap/phmap.h>
// #include "flat_hash_map.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <math.h>
#include <map>
#include <numeric>
#include <regex>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace trienewwords {

class WordStats;
class WordScore;

using count_t = size_t;
using word_t = std::string;
using position_t = unsigned int;
using adj_word_t = std::map<word_t, std::pair<count_t, count_t>>;
using word_stat_t = std::pair<count_t, adj_word_t>;
// using dict_t = ska::flat_hash_map<word_t, word_stat_t>;
// using dict_t = std::unordered_map<word_t, word_stat_t>;
// using dict_t = std::map<word_t, word_stat_t>;
// using dict_t = tsl::array_map<char, word_stat_t>;
using dict_t = tsl::htrie_map<char, word_stat_t>;
using score_pair_t = std::pair<word_t, WordScore>;
using score_list_t = std::vector<score_pair_t>;

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

class TrieNewWords {
public:
    TrieNewWords();
    TrieNewWords(const size_t max_gram, const size_t min_count, const float base_solidity, const float min_entropy);

    score_list_t discover(std::istream& inp_stream);
    
private:
    size_t max_gram;
    size_t min_count;
    std::vector<float> min_solidity;
    float min_entropy;

    dict_t getCandidateNgrams(std::istream& inp_stream);

    dict_t mergeNdicts(const std::vector<dict_t>& dict_list);

    std::vector<dict_t> splitNdicts(const dict_t& dict);

    score_list_t filteredDicts(const dict_t& dict);

    count_t getUnigramSum(const dict_t& dict);

    float solidity(const std::string& word, const dict_t& d);

    float entropy(const std::vector<count_t>& counts);

    float adjEntropy(const adj_word_t& adj_words);

};

}


#endif