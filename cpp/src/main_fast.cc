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

bool mycomp(const score_pair_t& s1, const score_pair_t& s2) {
    return s1.second.count > s2.second.count;
}

int main(int argc, char** argv) {
    string map_type;
    if (cmdOptionExists(argv, argv+argc, "-h") || cmdOptionExists(argv, argv+argc, "--help")) {
        cerr << 
        " - New words Discovery Module options\n"
        " Usage:\n"
        "   ./newwords-fast [OPTION...]\n\n"
        "   -m, --map_type arg       Mapping type, 'hash' or 'trie' (default: 'hash')\n"
        "   -g, --max_gram arg       Max gram length (default: 4)\n"
        "   -c, --min_count arg      Minimum count (default: 5)\n"
        "   -s, --base_solidity arg  Minimum solidity of unigram (default: 5.0)\n"
        "   -e, --min_entropy arg    Minimum entropy (default: 2.0)\n"
        "       --sort               Sort the results\n"
        "   -h, --help               Print help\n\n";
        return 0;
    }

    if (cmdOptionExists(argv, argv+argc, "-m")) {
        map_type = getCmdOption(argv, argv + argc, "-m");
    } else if (cmdOptionExists(argv, argv+argc, "--map_type")) {
        map_type = getCmdOption(argv, argv + argc, "--map_type");
    } else {
        map_type = "hash";
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
    
    bool sort_results = false;
    if (cmdOptionExists(argv, argv+argc, "--sort"))
        sort_results = true;

    cerr << std::fixed;
    cerr << "\nNew Words Discovery Parameters: " << endl;
    cerr << "\t[map_type]:      " << map_type << endl;
    cerr << "\t[max_gram]:      " << max_gram << endl;
    cerr << "\t[min_count]:     " << min_count << endl;
    cerr << "\t[base_solidity]: " << setprecision(1) << base_solidity << endl;
    cerr << "\t[min_entropy]:   " << setprecision(1) << min_entropy << endl;
    cerr << "\t[sort]:          " << string(sort_results ? "true": "false") << endl << endl;

    FastNewWords d(
        map_type, 
        max_gram,
        min_count,
        base_solidity,
        min_entropy
        );

    score_list_t scores = d.discover(std::cin);
    if (sort_results)
        sort(scores.begin(), scores.end(), mycomp);
    // output
    for (auto kv: scores) {
        std::cout << kv.first << " ";
        std::cout << kv.second.count << " ";
        std::cout << kv.second.solidity << " ";
        std::cout << kv.second.entropy << " " << std::endl;
    }

    return 0;
}