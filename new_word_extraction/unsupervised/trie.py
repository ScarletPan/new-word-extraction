# coding:utf-8
# Copyright (c) 2019, Tencent Inc.
# All rights reserved.
# Author: Haojie Pan <jasonhjpan@tencent.com>


class TrieNode(object):
    def __init__(self, char, depth=0):
        self.char = char
        self.depth = depth
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
        self.root = TrieNode('/')

    def insert(self, word):
        depth = 1
        node = self.root
        for ch in word:
            node.is_end = False
            if ch in node.children:
                node = node.children[ch]
            else:
                node.children[ch] = TrieNode(ch, depth)
                node = node.children[ch]
            depth += 1
        node.count += 1
        node.is_end = True

    def find(self, word):
        node = self.root
        for ch in word:
            if ch in node.children:
                node = node.children[ch]
        return node

    def merge(self, n1, n2):
        if n1 is None:
            return n2
        if n2 is None:
            return n1
        node = TrieNode(n1.char, n1.depth)
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


if __name__ == "__main__":
    from new_word_extraction.utils.utils import get_all_ngrams
    # doc = open("data/sample/sample.txt", encoding="utf-8").read()
    # ngrams = get_all_ngrams(list(doc), max_gram=5, sep='')
    ngrams = ["蔡英文", "台湾北部"]
    trie1 = TrieTree()
    for gram in ngrams:
        trie1.insert(gram)
    print(trie1.find("蔡英文"))

    ngrams = ["蔡英文", "台湾"]
    trie2 = TrieTree()
    for gram in ngrams:
        trie2.insert(gram)
    print(trie2.find("蔡英文"))
    print()

    trie1.join(trie2)
    print()