// Copyright (c) 2019-present, Tencent, Inc.
// All rights reserved.
// 
// Author: Haojie Pan
// Email: jasonhjpan@tencent.com
//


#ifndef NEWWORDS_FASTNEWWORDS_H_
#define NEWWORDS_FASTNEWWORDS_H_

#include <map>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "tsl/htrie_map.h"


namespace fastnewwords {

class WordStats;
class WordScore;

using count_t = size_t;
using word_t = std::string;
using word_set_t = std::unordered_set<word_t>;
using position_t = unsigned int;
using word_stat_t = std::pair<count_t, std::vector<position_t>>;
using dict_t = std::unordered_map<word_t, word_stat_t>;
using trie_t = tsl::htrie_map<char, word_stat_t>;
using score_list_t = std::vector<std::pair<word_t, WordScore>>;
using score_dict_t = std::map<word_t, std::pair<count_t, WordScore>>;

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
    FastNewWords(const std::string map_type, const size_t max_gram, 
                 const size_t min_count, const float base_solidity,  
                 const float min_entropy);

    void retrieve(std::istream& inp_stream, std::ostream& outp_stream);

    void rerank(std::istream& inp_stream, 
                std::ostream& outp_stream,
                const std::string& dict_path="",
                const std::string& stopwords_path="");
    
private:
    // Parameters
    std::string map_type;
    size_t max_gram;
    size_t min_count;
    std::vector<float> min_solidity;
    float min_entropy;

    // raw input stream container
    std::string utf8_content;
    std::string reversed_utf8_content;

    // Adaptive operators for dic_t and trie_t
    void insertWord(dict_t& d, const word_t& wd, const word_stat_t& ws);
    void insertWord(trie_t& d, const word_t& wd, const word_stat_t& ws);
    word_t getKey(dict_t::const_iterator& it);
    word_t getKey(trie_t::const_iterator& it);
    word_stat_t getValue(dict_t::const_iterator& it);
    word_stat_t getValue(trie_t::const_iterator& it);
    count_t getUnigramSum(const dict_t& dict);
    count_t getUnigramSum(const trie_t& dict);

    template<typename T>
    float solidity(const std::string& word, const T& d, const count_t total);

    float adjEntropy(const word_t& wd,
                     const std::vector<position_t>& positions,
                     const std::string& utf8_content,
                     const std::string& reversed_utf8_content,
                     bool left);

    // Two steps for new words discovery
    template<typename T>
    T getCandidateNgrams(std::istream& inp_stream);

    template<typename T>
    score_list_t filteredDicts(const T& dict);
};

} // namespace fastnewwords


#endif // NEWWORDS_FASTNEWWORDS_H_