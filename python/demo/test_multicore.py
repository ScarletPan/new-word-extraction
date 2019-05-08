# encoding: utf-8

import time
from new_word_extraction.unsupervised.infoEnt import MultiCoreInfoEntropyDiscoverer


if __name__ == "__main__":
    # text = open("../MazeKE/data/wechat/sample-19-03-18/bizmsg_sample_level2.sent.txt", encoding="utf8").read()
    text = open("data/books/tianlongbabu_jinyong.txt", encoding="utf-8").read()
    # texts = [''.join(t.split(' ')) for t in text.split('\n')]
    texts = text.split('\n')
    st = time.time()
    multicore_discoverer = MultiCoreInfoEntropyDiscoverer()
    multicore_discoverer.discover(texts)
    print("Finished in {:.2f} s\n".format(time.time() - st))
    print("=== [两字] ===")
    for item in multicore_discoverer.topK_frequent_words(topK=50, withWeight=False, min_length=2, max_length=2):
        print(item, end=' ')
    print()
    print("\n=== [三字] ===")
    for item in multicore_discoverer.topK_frequent_words(topK=50, withWeight=False, min_length=3, max_length=3):
        print(item, end=' ')
    print()
    print("\n=== [四字] ===")
    for item in multicore_discoverer.topK_frequent_words(topK=50, withWeight=False, min_length=4, max_length=4):
        print(item, end=' ')
    print()
