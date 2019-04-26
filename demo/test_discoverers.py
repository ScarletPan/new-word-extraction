# encoding: utf-8

from new_word_extraction.unsupervised.infoEnt import InfoEntropyDiscoverer


if __name__ == "__main__":
    text = open("data/books/tianlongbabu_jinyong.txt", encoding="utf-8").read()
    # text = open("data/books/renmingdemingyi.txt", encoding="utf-8").read()
    # text = open("data/books/bingyuhuo.txt", encoding="utf-8").read()
    discoverer = InfoEntropyDiscoverer()
    discoverer.discover([text])
    print("=== [两字] ===")
    for item in discoverer.topK_frequent_words(topK=50, withWeight=False, min_length=2, max_length=2):
        print(item, end=' ')
    print()
    print("\n=== [三字] ===")
    for item in discoverer.topK_frequent_words(topK=50, withWeight=False, min_length=3, max_length=3):
        print(item, end=' ')
    print()
    print("\n=== [四字] ===")
    for item in discoverer.topK_frequent_words(topK=50, withWeight=False, min_length=4, max_length=4):
        print(item, end=' ')
    print()