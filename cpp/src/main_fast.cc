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
    if (cmdOptionExists(argv, argv+argc, "-h") || cmdOptionExists(argv, argv+argc, "--help")) {
        cerr << 
        " - New words Discovery Module options\n"
        " Usage:\n"
        "   ./newwords-fast [OPTION...]\n\n"
        "   -M, --mode arg           program mode, 'retrieve' or 'rerank' (default: 'retrieve'\n"
        "   -m, --map_type arg       Mapping type, 'hash' or 'trie' (default: 'hash')\n"
        "   -g, --max_gram arg       Max gram length (default: 4)\n"
        "   -c, --min_count arg      Minimum count (default: 5)\n"
        "   -s, --base_solidity arg  Minimum solidity of unigram (default: 5.0)\n"
        "   -e, --min_entropy arg    Minimum entropy (default: 2.0)\n"
        "   -D, --dict arg           Exsiting dict path\n"
        "   -S, --stopwords arg      Stopwords path\n"
        "       --topk arg           output topk frequent words\n"
        "       --noscores arg       output words without scores\n"
        "   -h, --help               Print help\n\n";
        return 0;
    }


    string mode;
    if (cmdOptionExists(argv, argv+argc, "-M")) {
        mode = getCmdOption(argv, argv + argc, "-M");
    } else if (cmdOptionExists(argv, argv+argc, "--mode")) {
        mode = getCmdOption(argv, argv + argc, "--mode");
    } else {
        mode = "retrieve";
    }
    if (mode != "retrieve" && mode != "rerank") {
        cerr << "Please choose the valid mode : 'retrieve' or 'rerank' " << endl;
        return 0;
    }

    string map_type;
    if (cmdOptionExists(argv, argv+argc, "-m")) {
        map_type = getCmdOption(argv, argv + argc, "-m");
    } else if (cmdOptionExists(argv, argv+argc, "--map_type")) {
        map_type = getCmdOption(argv, argv + argc, "--map_type");
    } else {
        map_type = "hash";
    }
    if (map_type != "hash" && map_type != "trie") {
        cerr << "Please choose the valid map type : 'hash' or 'trie' " << endl;
        return 0;
    }

    size_t max_gram;
    if (cmdOptionExists(argv, argv+argc, "-g")) {
        max_gram = std::stoi(getCmdOption(argv, argv + argc, "-g"));
    } else if (cmdOptionExists(argv, argv+argc, "--max_gram")) {
        max_gram = std::stoi(getCmdOption(argv, argv + argc, "--max_gram"));
    } else {
        max_gram = 4;
    }

    size_t min_count;
    if (cmdOptionExists(argv, argv+argc, "-c")) {
        min_count = std::atoi(getCmdOption(argv, argv + argc, "-c"));
    } else if (cmdOptionExists(argv, argv+argc, "--min_count")) {
        min_count = std::atoi(getCmdOption(argv, argv + argc, "--min_count"));
    } else {
        min_count = 5;
    }

    float base_solidity;
    if (cmdOptionExists(argv, argv+argc, "-s")) {
        base_solidity = std::atof(getCmdOption(argv, argv + argc, "-s"));
    } else if (cmdOptionExists(argv, argv+argc, "--base_solidity")) {
        base_solidity = std::atof(getCmdOption(argv, argv + argc, "--base_solidity"));
    } else {
        base_solidity = 5.0;
    }

    float min_entropy;
    if (cmdOptionExists(argv, argv+argc, "-e")) {
        min_entropy = std::atof(getCmdOption(argv, argv + argc, "-e"));
    } else if (cmdOptionExists(argv, argv+argc, "--min_entropy")) {
        min_entropy = std::atof(getCmdOption(argv, argv + argc, "--min_entropy"));
    } else {
        min_entropy = 2.0;
    }

    string dict_path;
    if (cmdOptionExists(argv, argv+argc, "-D")) {
        dict_path = getCmdOption(argv, argv + argc, "-D");
    } else if (cmdOptionExists(argv, argv+argc, "--dict")) {
        dict_path = getCmdOption(argv, argv + argc, "--dict");
    } 
    
    string stopwords_path;
    if (cmdOptionExists(argv, argv+argc, "-S")) {
        stopwords_path = getCmdOption(argv, argv + argc, "-S");
    } else if (cmdOptionExists(argv, argv+argc, "--stopwords")) {
        stopwords_path = getCmdOption(argv, argv + argc, "--stopwords");
    } 

    int topk = -1;
    if (cmdOptionExists(argv, argv+argc, "--topk")) {
        topk = std::stoi(getCmdOption(argv, argv + argc, "--topk"));
    } 

    bool withscores = true;
    if (cmdOptionExists(argv, argv+argc, "--noscores")) {
        withscores = false;
    } 

    FastNewWords discoverer(map_type, 
                            max_gram,
                            min_count,
                            base_solidity,
                            min_entropy
                            );

    if (mode == "retrieve") {
        cerr << "====== Retrieving ======" << endl;
        cerr << std::fixed;
        cerr << "\nNew Words Discovery Parameters: " << endl;
        cerr << "\t[map_type]:      " << map_type << endl;
        cerr << "\t[max_gram]:      " << max_gram << endl;
        cerr << "\t[min_count]:     " << min_count << endl;
        cerr << "\t[base_solidity]: " << setprecision(1) << base_solidity << endl;
        cerr << "\t[min_entropy]:   " << setprecision(1) << min_entropy << endl;

        discoverer.retrieve(std::cin, std::cout);
    } else if (mode == "rerank") {
        cerr << "====== Reranking ======" << endl;
        FastNewWords d;
        discoverer.rerank(std::cin, std::cout, dict_path,  stopwords_path, topk, withscores);
    }

    return 0;
}