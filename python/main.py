import argparse
import sys
from new_word_extraction.unsupervised.infoEnt import MultiCoreInfoEntropyDiscoverer


def print_ngrams_and_neighbors(ngrams, gram2neighbors, filename=None):
    if filename is None:
        fout = sys.stdout
    else:
        fout = open(filename, "w+", encoding="utf8")
    for gram in ngrams:
        s = gram + '|'
        count = ngrams[gram]
        s += str(count) + '|'
        if gram not in gram2neighbors:
            s += '|'
        else:
            tmp_list = [key + '_' + str(val) for key, val in gram2neighbors[gram][0].items()]
            s += '$'.join(tmp_list) + '|'
            tmp_list = [key + '_' + str(val) for key, val in gram2neighbors[gram][1].items()]
            s += '$'.join(tmp_list)
        fout.write(s + '\n')
    if filename is not None:
        fout.close()

def load_ngrams_and_neighbors(inp_prefix, n_splits):
    ngram_counts, ngram_neighbors = [], []
    for i in range(n_splits):
        if n_splits > 1:
            fin = open(inp_prefix + str(i))
        else:
            fin = open(inp_prefix)
        for line in fin:
            word, count, tlt_d, trt_d = line.strip('\n').split('|')
            ngram_counts.append((word, int(count)))
            if tlt_d and trt_d:
                lt_d = dict()
                for item in tlt_d.split('$'):
                    char, count = item.split('_')
                    lt_d[char] = int(count)
                rt_d = dict()
                for item in trt_d.split('$'):
                    char, count = item.split('_')
                    rt_d[char] = int(count)
                ngram_neighbors.append((word, (lt_d, rt_d)))
        fin.close()
    return ngram_counts, ngram_neighbors

def print_ngrams_and_scores(ngrams):
    for gram, scores in ngrams.items():
        print(gram + '|' + '|'.join([str(t) for t in scores]))

def load_ngrams_and_scores(inp_prefix, n_splits):
    scores = []
    gram_set = set()
    for i in range(n_splits):
        fin = open(inp_prefix + str(i))
        for line in fin:
            if line.strip('\n'):
                gram, count, solidity, entropy = line.strip('\n').split('|')
                if gram in gram_set:
                    continue
                gram_set.add(gram)
                scores.append((gram, int(count), float(solidity), float(entropy)))
    return scores


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-generate_ngrams",
                        action="store_true",
                        help="generate ngrams")
    parser.add_argument("-merge_and_split_ngrams",
                        action="store_true",
                        help="merge and split ngrams")
    parser.add_argument("-selecting_candidates",
                        action="store_true",
                        help="select candidate words by solidity and entropy")
    parser.add_argument("-get_final_results",
                        action="store_true",
                        help="generate final results")
    parser.add_argument("-n_splits",
                        type=int,
                        default=5,
                        help="generate ngrams")
    parser.add_argument("-inp_prefix",
                        type=str,
                        default='./tmp.txt',
                        help="Input prefix")
    parser.add_argument("-outp_prefix",
                        type=str,
                        default='./tmp.txt',
                        help="generate ngrams")
    parser.add_argument("-inp",
                        type=str,
                        default='./tmp.txt',
                        help="generate ngrams")
    parser.add_argument("-outp",
                        type=str,
                        default='./tmp.txt',
                        help="generate ngrams")
    discoverer = MultiCoreInfoEntropyDiscoverer()
    opt = parser.parse_args()
    if opt.generate_ngrams:
        texts = [line.strip() for line in sys.stdin]
        ngrams, gram2neighbors = discoverer.get_canditate_ngrams_per_split(texts)
        print_ngrams_and_neighbors(ngrams, gram2neighbors)
    elif opt.merge_and_split_ngrams:
        ngram_counts, ngram_neighbors = load_ngrams_and_neighbors(inp_prefix=opt.inp_prefix, n_splits=opt.n_splits)
        ngrams, gram2neighbors = discoverer.merge_ngrams_and_neighbors(ngram_counts, ngram_neighbors)
        unigram_sum = sum([val for key, val in ngrams.items() if len(key) == 1])
        for i, (sub_ngrams, sub_gram2neighbors) in enumerate(
                discoverer.split_ngrams_and_neighbors(ngrams, gram2neighbors, n_split=opt.n_splits)):
            print_ngrams_and_neighbors(sub_ngrams, gram2neighbors, opt.outp_prefix + str(i))
    elif opt.selecting_candidates:
        ngram_counts, ngram_neighbors = load_ngrams_and_neighbors(opt.inp, n_splits=1)
        ngrams, gram2neighbors = discoverer.merge_ngrams_and_neighbors(ngram_counts, ngram_neighbors)
        ret_ngrams = discoverer.filtered_by_solidity_and_entropy(ngrams, gram2neighbors)
        print_ngrams_and_scores(ret_ngrams)
    elif opt.get_final_results:
        scores = load_ngrams_and_scores(opt.inp_prefix, n_splits=opt.n_splits)
        scores.sort(key=lambda x: x[1], reverse=True)
        for item in scores:
            line = '|'.join([str(t) for t in item]) + '\n'
            print(line, end='')

