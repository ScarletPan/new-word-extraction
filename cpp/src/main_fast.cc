// Copyright (c) 2019-present, Tencent, Inc.
// All rights reserved.
// 
// Author: Haojie Pan
// Email: jasonhjpan@tencent.com
//

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "fastnewwords.h"
#include "argparse.h"

using namespace fastnewwords;
using namespace std;


char* getCmdOption(char ** begin, char ** end, const std::string & option) {
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

int main(int argc, char** argv) {
    argparse::ArgumentParser parser("New words Discovery Module options");
    parser.add_argument("--mode", "string", "retrieve", 
                        "program mode, 'retrieve' or 'rerank' (default: 'retrieve')");
    parser.add_argument("--map_type", "string", "hash", 
                        "Mapping type, 'hash' or 'trie' (default: 'hash')");
    parser.add_argument("--max_gram", "int", "4", 
                        "Max gram length (default: 4)");
    parser.add_argument("--min_count", "int", "5", 
                        "Minimum count (default: 5)");
    parser.add_argument("--min_solidity", "float", "5.0", 
                        "Minimum solidity of unigram (default: 5.0)");
    parser.add_argument("--min_entropy", "float", "2.0", 
                        "Minimum entropy (default: 2.0)");
    parser.add_argument("--topk", "int", "20", 
                        "Return only topk after reranking (default: 20)");
    parser.add_argument("--dict", "string", "", 
                        "Exsiting dict path");
    parser.add_argument("--stopwords", "string", "", 
                        "Stopwords path");
    parser.add_argument("--noscores", "bool", "0", 
                        "output words without scores");
    if (!parser.parse_args(argc, argv))
        return 0;

    string mode, map_type, dict_path, stopwords_path;
    int max_gram, min_count, topk;
    float min_solidity, min_entropy;
    bool noscores;
    parser.get("mode", mode);
    parser.get("map_type", map_type);
    parser.get("max_gram", max_gram);
    parser.get("min_count", min_count);
    parser.get("min_solidity", min_solidity);
    parser.get("min_entropy", min_entropy);
    parser.get("topk", topk);
    parser.get("dict", dict_path);
    parser.get("stopwords", stopwords_path);
    parser.get("noscores", noscores);

    FastNewWords discoverer(map_type, 
                            max_gram,
                            min_count,
                            min_solidity,
                            min_entropy
                            );

    if (mode == "retrieve") {
        cerr << "====== Retrieving ======" << endl;
        cerr << std::fixed;
        cerr << "\nNew Words Discovery Parameters: " << endl;
        cerr << "\t[map_type]:      " << map_type << endl;
        cerr << "\t[max_gram]:      " << max_gram << endl;
        cerr << "\t[min_count]:     " << min_count << endl;
        cerr << "\t[min_solidity]: " << setprecision(1) << min_solidity << endl;
        cerr << "\t[min_entropy]:   " << setprecision(1) << min_entropy << endl;

        discoverer.retrieve(std::cin, std::cout);
    } else if (mode == "rerank") {
        cerr << "====== Reranking ======" << endl;
        FastNewWords d;
        discoverer.rerank(std::cin, std::cout, dict_path,  stopwords_path, topk, !noscores);
    }

    return 0;
}