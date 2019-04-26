from collections import defaultdict
from tqdm import tqdm
from new_word_extraction.constants import PUNCTUATIONS
from new_word_extraction.utils.stats import solidity, word_list_entropy


class InfoEntropyDiscoverer(object):
    """
        基于信息熵的新词发现方法
        参考 matrix67: http://www.matrix67.com/blog/archives/5044
    """
    def __init__(self, max_gram=4, min_freq=5, ngram_min_solidity=None, min_entropy=2.0):
        self.max_gram = max_gram
        self.min_freq = min_freq
        self.min_solidity = ngram_min_solidity if ngram_min_solidity else \
            {i: 5 ** i for i in range(1, max_gram + 1)}
        self.min_entropy = min_entropy
        self.ngrams = None
        self.gram2solidity = None
        self.gram2entropy = None

    def discover(self, doc_iter):
        # Find candidate ngrams
        ngrams = defaultdict(int)
        for text in doc_iter:
            for i in range(len(text)):
                for j in range(1, self.max_gram + 1):
                    if set(text[i: i + j]) & PUNCTUATIONS or text[i: i + j].strip() is None:
                        continue
                    if i + j < len(text):
                        ngrams[text[i: i + j]] += 1
        # Filtering low frequency ngrams
        self.ngrams = {key: val for key, val in ngrams.items() if val >= self.min_freq}
        unigram_sum = sum([val for key, val in self.ngrams.items() if len(key) == 1])

        # Filtering low solidity ngrams
        gram2solidity = {key: solidity(key, self.ngrams) * unigram_sum for key in self.ngrams}
        ngrams = {key: val for key, val in self.ngrams.items()
                  if gram2solidity[key] >= self.min_solidity[len(key)]}
        self.ngrams = {}

        # Filtering low entropy ngrams
        gram2neighbors = {}
        for text in doc_iter:
            for i in range(len(text)):
                for j in range(1, self.max_gram + 1):
                    _gram = text[i: i + j]
                    if _gram in ngrams:
                        if _gram not in gram2neighbors:
                            gram2neighbors[_gram] = [[], []]
                        if i > 0:
                            gram2neighbors[_gram][0].append(text[i - 1])
                        if i + j < len(text):
                            gram2neighbors[_gram][1].append(text[i + j])
        gram2entropy = {
            _gram:
            min(word_list_entropy(gram2neighbors[_gram][0]),
                word_list_entropy(gram2neighbors[_gram][1]))
            for _gram in ngrams}
        self.ngrams = {key: val for key, val in ngrams.items() if gram2entropy[key] >= self.min_entropy}
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
