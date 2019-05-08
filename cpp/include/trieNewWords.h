#ifndef __infoEnt_h__
#define __infoEnt_h__

#include "myutils.h"
#include <tsl/htrie_map.h>
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

    // Single Machine
    score_list_t discover(std::istream& inp_stream);

    // Map Reduce operators
    void map_1_getNgrams(std::istream& inp_stream, std::ostream& outp_stream);

    void sort_split_1_Ngrams(std::istream& inp_stream, std::ostream& outp_stream);

    void reduce_1_mergeNgrams(std::istream& inp_stream, std::ostream& outp_stream);

    void map_2_filtered_by_entropy(std::istream& inp_stream, std::ostream& outp_stream);

    void sort_split_2_Ngrams(std::istream& inp_stream, std::ostream& outp_stream);

    void reduce_2_mergeNgrams(std::istream& inp_stream, std::ostream& outp_stream);
    
private:
    size_t max_gram;
    size_t min_count;
    std::vector<float> min_solidity;
    float min_entropy;

    dict_t getCandidateNgrams(std::istream& inp_stream, bool filtered_min_freq=true);

    score_list_t filteredDicts(const dict_t& dict);

    count_t getUnigramSum(const dict_t& dict);

    float solidity(const word_t& word, const dict_t& d);

    float entropy(const std::vector<count_t>& counts);

    float adjEntropy(const adj_word_t& adj_words);

    std::string wordStat2str(const word_t& wd, const word_stat_t& ws);

};

}


#endif