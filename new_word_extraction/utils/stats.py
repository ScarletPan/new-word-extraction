import numpy as np
import scipy.stats

def solidity(word, word2prob):
    """

        solidity(电影院) = p(电影院) / max{p(电, 影院}, p(电影, 院)}

    :param word: str
    :param word2prob: dict
    :return: float
    """
    if len(word) == 1:
        return float('inf')
    coocurrences = [
        (word[:i], word[i:], word2prob[word[:i]] * word2prob[word[i:]])
         for i in range(1, len(word))
         if word[:i] in word2prob and word[i:] in word2prob]
    if not coocurrences:
        return 0
    max_coocurrence = max(coocurrences, key=lambda x: x[2])
    return word2prob[word] / max_coocurrence[-1]


def word_list_entropy(word_list=None, counts=None, base=None):
    """
        word_list_entropy([w1, w2, w2, ..., w1, ...])
            = -log p(w1) - log p(w2) ...

    :param word_list: list of str
    :param base: 2 or e
    :return: float
    """
    if counts is None:
        if word_list is None:
            return 0.0
        values, counts = np.unique(word_list, return_counts=True)
    return scipy.stats.entropy(counts, base=base)
