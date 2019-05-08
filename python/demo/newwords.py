# encoding: utf-8

import argparse
import time
import sys
from new_word_extraction.unsupervised.infoEnt import InfoEntropyDiscoverer


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-g", "--max_gram", type=int, default=4,
                        help="Max Length of gram")
    parser.add_argument("-c", "--min_count", type=int, default=5,
                        help="Minimum occurrence of the gram")
    parser.add_argument("-s", "--base_solidity", type=int, default=5.0,
                        help="Minimum solidity of unigram")
    parser.add_argument("-e", "--min_entropy", type=int, default=2.0,
                        help="Minimum entropy")

    opt = parser.parse_args()

    text = sys.stdin.read()
    st = time.time()
    discoverer = InfoEntropyDiscoverer(max_gram=opt.max_gram,
                                       min_freq=opt.min_count,
                                       ngram_min_solidity={
                                           i: opt.base_solidity ** i for i in range(1, opt.max_gram + 1)},
                                       min_entropy=opt.min_entropy
                                       )
    discoverer.discover(text.split('\n'))
    sys.stderr.write("Finished in {:.2f} s\n".format(time.time() - st))
    for key, item in discoverer.topK_frequent_words(topK=None, withWeight=True, min_length=2, max_length=5):
        print(' '.join([key, str(item["frequency"]), str(item["solidity"]), str(item["entropy"])]))
