#ifndef __infoEnt_h__
#define __infoEnt_h__

#include "myutils.h"
#include <tsl/htrie_map.h>
#include <iostream>
#include <iomanip>
#include <limits>
#include <math.h>
#include <numeric>
#include <regex>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fastnewwords {

class WordStats;
class WordScore;

using count_t = size_t;
using word_t = std::string;
using position_t = unsigned int;
using word_stat_t = std::pair<count_t, std::vector<position_t>>;
using dict_t = std::unordered_map<word_t, word_stat_t>;
using trie_t = tsl::htrie_map<char, word_stat_t>;
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

class FastNewWords {
public:
    FastNewWords();
    FastNewWords(const std::string map_type, const size_t max_gram, const size_t min_count, const float base_solidity, const float min_entropy);

    score_list_t discover(std::istream& inp_stream);
    
private:
    std::string map_type;
    size_t max_gram;
    size_t min_count;
    std::vector<float> min_solidity;
    float min_entropy;
    std::string utf8_content;
    std::string reversed_utf8_content;

    void insertWord(dict_t& d, const word_t& wd, const word_stat_t& ws);

    void insertWord(trie_t& d, const word_t& wd, const word_stat_t& ws);

    word_t getKey(dict_t::const_iterator& it);

    word_t getKey(trie_t::const_iterator& it);

    word_stat_t getValue(dict_t::const_iterator& it);

    word_stat_t getValue(trie_t::const_iterator& it);

    template<typename T>
    T getCandidateNgrams(std::istream& inp_stream);

    template<typename T>
    score_list_t filteredDicts(const T& dict);
    
    count_t getUnigramSum(const dict_t& dict);

    count_t getUnigramSum(const trie_t& dict);

    template<typename T>
    float solidity(const std::string& word, const T& d);

    float entropy(const word_t& wd,
                  const std::vector<position_t>& positions,
                  const std::string& utf8_content,
                  const std::string& reversed_utf8_content,
                  bool left);
};

}


#endif