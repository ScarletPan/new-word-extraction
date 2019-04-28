# coding:utf-8
# Copyright (c) 2019, Tencent Inc.
# All rights reserved.
# Author: Haojie Pan <jasonhjpan@tencent.com>


from collections import defaultdict
import numpy as np
from new_word_extraction.constants import PUNCTUATIONS
from new_word_extraction.unsupervised.trie import TrieTree
from new_word_extraction.utils.stats import solidity, word_list_entropy
from new_word_extraction.utils.utils import get_all_ngrams


class InfoEntropyDiscoverer(object):
    """
        基于信息熵的新词发现方法
        参考 matrix67: http://www.matrix67.com/blog/archives/5044

        dict版实现
    """
    def __init__(self, max_gram=4, min_freq=5, ngram_min_solidity=None, min_entropy=2.0):
        self.max_gram = max_gram
        self.min_freq = min_freq
        self.min_solidity = ngram_min_solidity if ngram_min_solidity else \
            {i: 5 ** i for i in range(1, max_gram + 1)}
        self.min_entropy = min_entropy
        self.unigram_sum = 0
        self.ngrams = None
        self.gram2solidity = None
        self.gram2entropy = None

    def find_candidate_ngrams(self, text):
        # Find candidate ngrams
        ngrams = defaultdict(int)
        for i in range(len(text) - self.max_gram):
            for j in range(1, self.max_gram + 1):
                if text[i + j - 1] in PUNCTUATIONS:
                    break
                ngrams[text[i: i + j]] += 1
        for gram in get_all_ngrams(text[-self.max_gram:], sep=''):
            if not set(gram) & PUNCTUATIONS:
                ngrams[gram] += 1

        # Filtering low frequency ngrams
        ngrams = {key: val for key, val in ngrams.items() if val >= self.min_freq}
        self.unigram_sum = sum([val for key, val in ngrams.items() if len(key) == 1])

        return ngrams

    def filtered_by_solidity(self, ngrams):

        # Filtering low solidity ngrams
        gram2solidity = {key: solidity(key, ngrams) * self.unigram_sum for key in ngrams}
        ngrams = {key: val for key, val in ngrams.items()
                  if gram2solidity[key] >= self.min_solidity[len(key)]}
        return gram2solidity, ngrams

    def filtered_by_entropy(self, text, ngrams):
        # Filtering low entropy ngrams
        gram2neighbors = {}
        for i in range(1, len(text)):
            for j in range(1, self.max_gram + 1):
                _gram = text[i: i + j]
                if _gram in ngrams:
                    if i + j >= len(text):
                        break
                    if _gram not in gram2neighbors:
                        gram2neighbors[_gram] = [defaultdict(int), defaultdict(int)]
                    gram2neighbors[_gram][0][text[i - 1]] += 1
                    gram2neighbors[_gram][1][text[i + j]] += 1
        tmp_gram2entropy = {
            _gram: word_list_entropy(counts=list(gram2neighbors[_gram][0].values()))
            for _gram in ngrams}
        ngrams = {key: val for key, val in ngrams.items() if tmp_gram2entropy[key] >= self.min_entropy}
        gram2entropy = {
            _gram:
                min(tmp_gram2entropy[_gram],
                    word_list_entropy(counts=list(gram2neighbors[_gram][1].values())))
            for _gram in ngrams}
        ngrams = {key: val for key, val in ngrams.items() if gram2entropy[key] >= self.min_entropy}
        return gram2entropy, ngrams

    def discover(self, text):
        ngrams = self.find_candidate_ngrams(text)
        gram2solidity, ngrams = self.filtered_by_solidity(ngrams)
        gram2entropy, ngrams = self.filtered_by_entropy(text, ngrams)

        self.ngrams = ngrams
        self.gram2solidity = {key: gram2solidity[key] for key in self.ngrams}
        self.gram2entropy = {key: gram2entropy[key] for key in self.ngrams}

    def topK_frequent_words(self, topK=20, withWeight=False, min_length=2, max_length=5):
        tmp = [t for t in
               sorted(self.ngrams.items(), key=lambda x: x[1], reverse=True)
               if len(t[0]) >= min_length and len(t[0]) <= max_length]
        if topK:
            tmp = tmp[:topK]
        if not withWeight:
            return [t[0] for t in tmp]
        else:
            return [(gram,
                     {
                         "frequency": freq,
                         "solidity": self.gram2solidity[gram],
                         "entropy": self.gram2entropy[gram]
                     })
                    for gram, freq in tmp]


class TrieInfoEntropyDiscoverer(object):
    """
        基于信息熵的新词发现方法
        参考 matrix67: http://www.matrix67.com/blog/archives/5044

        Trie树实现
    """
    def __init__(self, max_gram=5, min_count=5, ngram_min_solidity=None, min_entropy=2.0):
        self.max_gram = max_gram
        self.min_count = min_count
        self.min_solidity = ngram_min_solidity if ngram_min_solidity else \
            {i: i * np.log(5) for i in range(1, max_gram + 1)}
        self.min_entropy = min_entropy
        self.ngrams = dict()
        self.gram2solidity = dict()
        self.gram2entropy = dict()

    def build_tries(self, text):
        prefix_trie = TrieTree()
        suffix_trie = TrieTree()
        for i in range(len(text)):
            for j in range(1, self.max_gram + 1):
                if set(text[i: i + j]) & PUNCTUATIONS or text[i: i + j].strip() is None:
                    continue
                if i + j < len(text):
                    word = text[i: i + j]
                    prefix_trie.insert(word)
                    suffix_trie.insert(word[::-1])

        return prefix_trie, suffix_trie

    def discover(self, text):
        from tqdm import tqdm
        # Insert ngrams into Trie
        prefix_trie, suffix_trie = self.build_tries(text)

        # Filtering low frequency ngrams
        for word, node in tqdm(prefix_trie):
            count = node.count
            if count < self.min_count:
                continue
            if len(word) == 4:
                a = 1
            solidity = prefix_trie.get_pmi(word)
            if solidity < self.min_solidity[len(word)]:
                continue
            entropy = min(prefix_trie.get_right_entropy(word),
                          suffix_trie.get_right_entropy(word[::-1]))
            if entropy < self.min_entropy:
                continue
            self.ngrams[word] = count
            self.gram2solidity[word] = solidity
            self.gram2entropy[word] = entropy

    def topK_frequent_words(self, topK=20, withWeight=False, min_length=2, max_length=5):
        tmp = [t for t in
               sorted(self.ngrams.items(), key=lambda x: x[1], reverse=True)
               if len(t[0]) >= min_length and len(t[0]) <= max_length]
        if topK:
            tmp = tmp[:topK]
        if not withWeight:
            return [t[0] for t in tmp]
        else:
            return [(gram,
                     {
                         "frequency": freq,
                         "solidity": self.gram2solidity[gram],
                         "entropy": self.gram2entropy[gram]
                     })
                    for gram, freq in tmp]