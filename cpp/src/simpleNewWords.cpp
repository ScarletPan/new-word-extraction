#include "simpleNewWords.h"

namespace simplenewwords {

SimpleNewWords::SimpleNewWords() {
    this->max_gram = 4;
    this->min_count = 5;
    this->min_solidity.push_back(1);
    for (size_t i = 1; i <= max_gram; ++i) {
        this->min_solidity.push_back(this->min_solidity[i - 1] * 5);
    }
    this->min_entropy = 2.0;
}


SimpleNewWords::SimpleNewWords(const size_t max_gram, const size_t min_count, const float base_solidity, const float min_entropy) {
    this->max_gram = max_gram;
    this->min_count = min_count;
    this->min_solidity.push_back(1);
    for (size_t i = 1; i <= max_gram; ++i) {
        this->min_solidity.push_back(this->min_solidity[i - 1] * base_solidity);
    }
    this->min_entropy = min_entropy;
}


dict_t SimpleNewWords::getCandidateNgrams(sentence_t& sentences) {
    int steps = 0, total_steps = sentences.size();
    wn2positions_t wn2positions;
    for (size_t i = 0; i < sentences.size(); ++i) {
        steps += 1;
        if (steps % 500 == 0) {
            std::cerr << std::fixed;
            std::cerr << "Get candidate ngrams step 1: " 
                    << std::setprecision(1) << std::setw(5) 
                    << 1.0 * steps / total_steps * 100 << "%" << " \r";
            std::cerr << std::flush;
        }
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

    std::cerr << "Get candidate ngrams step 1: 100%." << std::endl;

    steps = 0; total_steps = wn2positions.size();
    dict_t dict;
    wn2positions_t::iterator it;
    for (it = wn2positions.begin(); it != wn2positions.end(); it++ ) {
        steps += 1;
        if (steps % 500 == 0) {
            std::cerr << std::fixed;
            std::cerr << "Get candidate ngrams step 2: " 
                    << std::setprecision(1) << std::setw(5) 
                    << 1.0 * steps / total_steps * 100 << "%" << " \r";
            std::cerr << std::flush;
        }
        if (it->second.size() < this->min_count)
            continue;
        if (myutils::have_punk(it->first))
            continue;
        if (!myutils::is_chinese(it->first))
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
    std::cerr << "Get candidate ngrams step 2: 100%." << std::endl;
    return dict;
}


score_vec_t SimpleNewWords::filteredDicts(dict_t& dict) {
    score_dict_t score_dict;
    count_t uni_cnt = getUnigramSum(dict);
    int steps = 0, total_steps = dict.size();
    for (auto& kv: dict) {
        steps += 1;
        if (steps % 100 == 0) {
            std::cerr << std::fixed;
            std::cerr << "Filterred by solidity & entropy: " 
                    << std::setprecision(1) << std::setw(5) 
                    << 1.0 * steps / total_steps * 100 << "%" << " \r";
            std::cerr << std::flush;
        }
        if (kv.first.length() < 2)
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
    std::cerr << "Filterred by solidity & entropy: 100%" << std::endl;
    return score_vec_t(score_dict.begin(), score_dict.end());
}


count_t SimpleNewWords::getUnigramSum(dict_t& dict) {
    count_t cnt = 0;
    for (auto& kv: dict) {
        cnt += kv.second.count;
    }
    return cnt;
}


float SimpleNewWords::solidity(const std::wstring& word, const dict_t& d) {
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


float SimpleNewWords::entropy(adj_words_t& adj_words) {
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


score_vec_t SimpleNewWords::discover(sentence_t& sentences) {
    dict_t dict = getCandidateNgrams(sentences);
    score_vec_t score_vec = filteredDicts(dict);
    return score_vec;
}

}
