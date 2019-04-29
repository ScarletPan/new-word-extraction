# coding:utf-8
# Copyright (c) 2019, Tencent Inc.
# All rights reserved.
# Author: Haojie Pan <jasonhjpan@tencent.com>


from collections import defaultdict
import re
import time
import numpy as np
from tqdm import tqdm as tqdm
from new_word_extraction.constants import PUNCTUATIONS
from new_word_extraction.unsupervised.trie import TrieTree
from new_word_extraction.utils.stats import solidity, word_list_entropy
from new_word_extraction.utils.utils import get_all_ngrams, merge_dicts, split_list


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

    def get_gram_chars(self, text, i, j):
        return text[i: i + j]

    def get_gram_words(self, text, i, j):
        return tuple(text[i: i + j])

    def find_candidate_ngrams(self, texts):
        # Find candidate ngrams
        get_gram = self.get_gram_chars if isinstance(texts[0], str) else self.get_gram_words
        ngrams = defaultdict(int)
        gram2neighbors = dict()
        meaningless_token = frozenset(PUNCTUATIONS + '\n\t\u3000\xa0')
        for text in tqdm(texts):
            text += ' ' * self.max_gram
            for i in range(len(text) - self.max_gram):
                for j in range(1, self.max_gram + 1):
                    if text[i + j - 1] in meaningless_token:
                        break
                    _gram = get_gram(text, i, j)
                    ngrams[_gram] += 1
                    if _gram not in gram2neighbors:
                        gram2neighbors[_gram] = [defaultdict(int), defaultdict(int)]
                    gram2neighbors[_gram][0][text[i - 1]] += 1
                    gram2neighbors[_gram][1][text[i + j]] += 1
        # Filtering low frequency ngrams and none chinese character
        pattern = re.compile(r'[^\u4e00-\u9fa5]')
        tmp = []
        for key, val in ngrams.items():
            if not re.match(pattern, key) and val >= self.min_freq:
                tmp.append((key, val))
        ngrams = dict(tmp)
        gram2neighbors = {key: val for key, val in gram2neighbors.items() if key in ngrams}
        self.unigram_sum = sum([val for key, val in ngrams.items() if len(key) == 1])

        return ngrams, gram2neighbors

    def filtered_by_solidity(self, ngrams):
        # Filtering low solidity ngrams
        gram2solidity = {key: solidity(key, ngrams) * self.unigram_sum for key in ngrams}
        ngrams = {key: val for key, val in ngrams.items()
                  if gram2solidity[key] >= self.min_solidity[len(key)]}
        return ngrams, gram2solidity

    def filtered_by_entropy(self, ngrams, gram2neighbors):
        # Filtering low entropy ngrams
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
        ngrams, gram2neighbors = self.find_candidate_ngrams(text)
        ngrams, gram2solidity  = self.filtered_by_solidity(ngrams)
        gram2neighbors = {key: val for key, val in gram2neighbors.items() if key in ngrams}
        gram2entropy, ngrams = self.filtered_by_entropy(ngrams, gram2neighbors)

        self.ngrams = ngrams
        self.gram2solidity = {key: gram2solidity[key] for key in self.ngrams}
        self.gram2entropy = {key: gram2entropy[key] for key in self.ngrams}

    def topK_frequent_words(self, topK=20, withWeight=False, min_length=2, max_length=5):
        tmp = sorted(self.ngrams.items(), key=lambda x: x[1], reverse=True)
        if not tmp:
            return []
        if isinstance(tmp[0][0], tuple):
            res = [(''.join(t[0]), t[1]) for t in tmp]
        else:
            res = tmp
        res = [t for t in res
               if len(t[0]) >= min_length and len(t[0]) <= max_length]
        if topK:
            res = res[:topK]
        if not withWeight:
            return [t[0] for t in res]
        else:
            return [(gram,
                     {
                         "frequency": freq,
                         "solidity": self.gram2solidity[gram],
                         "entropy": self.gram2entropy[gram]
                     })
                    for gram, freq in res]


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


class MultiCoreInfoEntropyDiscoverer(InfoEntropyDiscoverer):
    def __init__(self, max_gram=4, min_freq=5, ngram_min_solidity=None, min_entropy=2.0):
        super().__init__(max_gram, min_freq, ngram_min_solidity, min_entropy)

    def get_canditate_ngrams_per_split(self, texts):
        """
        :param text: list of str
        :param file_path:
        :return: ngrams: dict, ngram => count
        """
        get_gram = self.get_gram_chars if isinstance(texts[0], str) else self.get_gram_words
        ngrams = defaultdict(int)
        gram2neighbors = dict()
        meaningless_token = frozenset(PUNCTUATIONS + '\n\t\u3000\xa0 ')
        for text in tqdm(texts):
            text += ' ' * self.max_gram
            for i in range(len(text) - self.max_gram):
                for j in range(1, self.max_gram + 1):
                    if text[i + j - 1] in meaningless_token:
                        break
                    _gram = get_gram(text, i, j)
                    ngrams[_gram] += 1
                    if _gram not in gram2neighbors:
                        gram2neighbors[_gram] = [defaultdict(int), defaultdict(int)]
                    gram2neighbors[_gram][0][text[i - 1]] += 1
                    gram2neighbors[_gram][1][text[i + j]] += 1
        # Filtering low frequency ngrams and none chinese character
        pattern = re.compile(r'[^\u4e00-\u9fa5]')
        tmp = []
        for key, val in ngrams.items():
            if not re.match(pattern, key):
                tmp.append((key, val))
        ngrams = dict(tmp)
        gram2neighbors = {key: val for key, val in gram2neighbors.items() if key in ngrams}

        return ngrams, gram2neighbors

    def merge_ngrams_and_neighbors(self, ngram_counts=None, ngram_neighbors=None):
        ngrams = dict()
        old_gram, cnt = None, 0
        for gram, count in ngram_counts:
            if old_gram == gram:
                cnt += count
            else:
                if cnt >= 2:
                    ngrams[old_gram] = cnt
                cnt = count
            old_gram = gram
        ngrams[old_gram] = cnt

        gram2neighbors = dict()
        for gram, (lt_d, rt_d) in ngram_neighbors:
            if gram not in ngrams:
                continue
            if gram not in gram2neighbors:
                gram2neighbors[gram] = [{}, {}]
            gram2neighbors[gram][0] = merge_dicts([lt_d, gram2neighbors[gram][0]])
            gram2neighbors[gram][1] = merge_dicts([rt_d, gram2neighbors[gram][1]])

        return ngrams, gram2neighbors

    def split_ngrams_and_neighbors(self, ngrams, gram2neighbors, n_split=10):
        unigram_sum = sum([val for key, val in ngrams.items() if len(key) == 1])
        ngrams_keys = list(sorted(ngrams.keys()))
        for keys in split_list(ngrams_keys, n_split):
            sub_ngrams = dict()
            sub_gram2neighbors = dict()
            added_ngram_keys = set()
            for word in keys:
                sub_gram2neighbors[word] = gram2neighbors[word]
                added_ngram_keys.add(word)
                for i in range(1, len(word)):
                    lt, rt = word[:i], word[i:]
                    added_ngram_keys.add(lt)
                    added_ngram_keys.add(rt)
            for add_key in added_ngram_keys:
                if add_key in ngrams:
                    sub_ngrams[add_key] = ngrams[add_key]
            sub_ngrams['unigram_sum'] = unigram_sum
            yield sub_ngrams, sub_gram2neighbors

    def filtered_by_solidity_and_entropy(self, ngrams, gram2neighbors):
        unigram_sum = ngrams['unigram_sum']
        gram2solidity = {key: solidity(key, ngrams) * unigram_sum for key in gram2neighbors}
        tmp_gram2entropy = {
            key: word_list_entropy(counts=list(gram2neighbors[key][0].values()))
            for key in gram2neighbors if gram2solidity[key] >= self.min_solidity[len(key)]}
        tmp_gram2entropy = {key: val for key, val in tmp_gram2entropy.items() if val >= self.min_entropy}
        gram2entropy = {
            key:
                min(tmp_gram2entropy[key],
                    word_list_entropy(counts=list(gram2neighbors[key][1].values())))
            for key in tmp_gram2entropy}
        gram2entropy = {key: val for key, val in gram2entropy.items() if val >= self.min_entropy}
        res = {gram: (ngrams[gram], gram2solidity[gram], gram2entropy[gram])
               for gram in gram2entropy if len(gram) >= 2}
        return res

    def merge_final_results(self, ngrams_list):
        ngrams = dict()
        for d in ngrams_list:
            for key, val in d.items():
                ngrams[key] = val
        return ngrams

    def discover(self, texts_list, n_split=4):
        st = time.time()
        print("Map => generating ngrams...")
        ret_list = []
        for texts in split_list(texts_list, n_split):
            res = self.get_canditate_ngrams_per_split(texts)
            ret_list.append(res)

        ngram_counts, ngram_neighbors = [], []
        for ngrams, gram2neighbors in ret_list:
            for key, val in ngrams.items():
                ngram_counts.append((key, val))
            for key, val in gram2neighbors.items():
                ngram_neighbors.append((key, val))
        ngram_counts.sort(key=lambda x: x[0])
        ngram_neighbors.sort(key=lambda x: x[0])
        print("Finished in {:.2f} s".format(time.time() - st))


        st = time.time()
        print("Reduce => merge ngrams and neighbors...")
        ngrams, gram2neighbors = self.merge_ngrams_and_neighbors(ngram_counts, ngram_neighbors)
        print("Finished in {:.2f} s".format(time.time() - st))

        st = time.time()
        print("Map => filtered using solidity and entropy...")
        ret_list = []
        for sub_ngrams, sub_gram2neighbors in self.split_ngrams_and_neighbors(ngrams, gram2neighbors):
            res = self.filtered_by_solidity_and_entropy(sub_ngrams, sub_gram2neighbors)
            ret_list.append(res)
        ngrams_list = ret_list
        print("Finished in {:.2f} s".format(time.time() - st))

        st = time.time()
        print("Reduce => merge final results")
        self.ngrams = self.merge_final_results(ngrams_list)
        print("Finished in {:.2f} s".format(time.time() - st))

    def topK_frequent_words(self, topK=20, withWeight=False, min_length=2, max_length=5):
        tmp = [t for t in
               sorted(self.ngrams.items(), key=lambda x: x[1][0], reverse=True)
               if len(t[0]) >= min_length and len(t[0]) <= max_length]
        if topK:
            tmp = tmp[:topK]
        if not withWeight:
            return [t[0] for t in tmp]
        else:
            return [(gram,
                     {
                         "count": c,
                         "solidity": s,
                         "entropy": e
                     })
                    for gram, (c, s, e) in tmp]

