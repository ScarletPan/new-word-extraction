def get_ngrams(_list, n=2, sep=None):
    res = zip(*[_list[j:] for j in range(n)])
    if sep is not None:
       res = [sep.join(t) for t in res]
    return res

def get_all_ngrams(_list, max_gram=5, sep=None):
    result = []
    for i in range(1, max_gram + 1):
        result.extend(get_ngrams(_list, i, sep))
    return result