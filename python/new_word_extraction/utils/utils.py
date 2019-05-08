from collections import defaultdict


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


def merge_dicts(dicts, _type=int):
    merged = defaultdict(_type)
    for d in dicts:
        for k in d:
            merged[k] += d[k]
    return merged


def split_list(_list, n_splits):
    avg = len(_list) / float(n_splits)
    last = 0.0

    while last < len(_list):
        yield _list[int(last):int(last + avg)]
        last += avg