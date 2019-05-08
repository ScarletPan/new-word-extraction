# coding:utf-8
# Copyright (c) 2019, Tencent Inc.
# All rights reserved.
# Author: Haojie Pan <jasonhjpan@tencent.com>


import numpy as np
from new_word_extraction.utils.stats import word_list_entropy


class TrieNode(object):
    def __init__(self):
        self.count = 0
        self.is_end = False
        self.children = dict()

    def __str__(self):
        s = "{}, depth {}, count {}".format(self.char, self.depth, self.count)
        if self.is_end:
            s += " [end]"
        return s

    def __repr__(self):
        return self.__str__()


class TrieTree(object):
    def __init__(self):
        self.root = TrieNode()
        self.unigram_count = 0

    def insert(self, word, count=1):
        depth = 1
        node = self.root
        for ch in word:
            if ch in node.children:
                node = node.children[ch]
            else:
                node.children[ch] = TrieNode()
                node = node.children[ch]
        node.count += count
        if depth == 1:
            self.unigram_count += count
        node.is_end = True

    def exist(self, word):
        return self.find(word) is not None

    def find(self, word):
        node = self.root
        for ch in word:
            if ch in node.children:
                node = node.children[ch]
            else:
                return None
        return node

    def find_all_next_chars(self, word, withCount=False):
        node = self.find(word)
        if node is not None and len(node.children) > 0:
            if withCount:
                return list(zip(*[(ch, child.count) for ch, child in node.children.items()]))
            else:
                return list(node.children.keys())
        else:
            if withCount:
                return None, None
            return None

    def get_count(self, word):
        node = self.find(word)
        return node.count

    def get_pmi(self, word):
        """
        pmi = log (p(电影院) / max{p(电) * p(影院), p(电影) * p(院)})
            = log (c(电影院) * N / max{c(电) * c(影院), c(电影) * c(院)})
            = log c(电影院) + log N - (log c1 + log c2)

        log is for avoiding overflow/underflow

        """
        if not self.exist(word):
            return float('-inf')
        if len(word) == 1:
            return float('inf')
        coocurrences = [
            (word[:i], word[i:], self.get_count(word[:i]), self.get_count(word[i:]))
            for i in range(1, len(word))
            if self.exist(word[:i]) and self.exist(word[i:])]
        if not coocurrences:
            return 0
        *_, cnt1, cnt2 = max(coocurrences, key=lambda x: x[2] * x[3])
        N = self.unigram_count
        return np.log(self.get_count(word)) + np.log(N) - (np.log(cnt1) + np.log(cnt2))

    def get_right_entropy(self, word):
        values, counts = self.find_all_next_chars(word, withCount=True)
        return word_list_entropy(counts=counts)

    def merge(self, n1, n2):
        if n1 is None:
            return n2
        if n2 is None:
            return n1
        node = TrieNode()
        node.is_end = n1.is_end or n2.is_end
        node.count = n1.count + n2.count
        all_child_chars = set(n1.children) | set(n2.children)
        for ch in all_child_chars:
            if ch in n1.children and ch in n2.children:
                node.children[ch] = self.merge(n1.children[ch], n2.children[ch])
            elif ch in n1.children:
                node.children[ch] = n1.children[ch]
            else:
                node.children[ch] = n2.children[ch]
        return node

    def join(self, another):
        self.root = self.merge(self.root, another.root)


    def __iter__(self):
        def dfs(node, prefix):
            if node is None:
                return
            for ch, child in node.children.items():
                if child.is_end:
                    yield prefix + ch
                dfs(child, prefix + ch)
        stk = list()
        stk.append((self.root, ''))
        while stk:
            node, word = stk.pop()
            if node.is_end:
                yield word, node
            for ch, child in node.children.items():
                stk.append((child, word + ch))


if __name__ == "__main__":
    from new_word_extraction.utils.utils import get_all_ngrams
    # doc = open("data/sample/sample.txt", encoding="utf-8").read()
    doc = "我在数据应用组做大数据挖掘，涵盖了大数应用"
    ngrams = get_all_ngrams(list(doc), max_gram=5, sep='')
    trie1 = TrieTree()
    for gram in ngrams:
        trie1.insert(gram)
    print(trie1.find_all_next_chars("数据"))
    print(len(doc))
    print(trie1.unigram_count)
    print(trie1.get_pmi("大数应用"))
    print(trie1.get_right_entropy("数据"))
    print([t[0] for t in trie1])

    ngrams = get_all_ngrams(list(reversed(doc)), max_gram=5, sep='')
    trie2 = TrieTree()
    for gram in ngrams:
        trie2.insert(gram)
    print(trie2.find_all_next_chars("据数"))
    print()