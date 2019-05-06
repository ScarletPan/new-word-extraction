#include "simpleNewWords.h"
#include <codecvt>
#include <clocale>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace simplenewwords;


bool mycomp(score_pair_t& s1, score_pair_t& s2) {
    return s1.second.count > s2.second.count;
}

void print_results(score_vec_t& scores, const int n, const int topk=50) {
    int cnt = 0;
    for (int i = 0; i < scores.size(); ++i) {
        auto kv = scores[i];
        if (kv.first.length() == n) {
            std::wcout << kv.first << " ";
            // std::wcout << kv.second.count << " ";
            // std::wcout << kv.second.solidity << std::setprecision(2) << " ";
            // std::wcout << kv.second.entropy << std::setprecision(2) << " " << std::endl;
            if (++cnt == topk)
                return;
        }
    }
    std::wcout << std::endl;
}

int main(int argc, char** argv) {
    // set encoding
    std::locale::global(std::locale("en_US.utf8"));
    std::wcout.imbue(std::locale("en_US.utf8"));

    // set args
    const std::string INPUT_FILE = argv[1];
    const std::string OUTPUT_FILE = argv[2];

    // get inputs
    std::wifstream infile(INPUT_FILE, std::ios::in);
    infile.imbue(std::locale("en_US.utf8"));

    std::vector<std::wstring> sentences;
    for (std::wstring line; infile >> line;) {
        sentences.push_back(line);
    }
    infile.close();

    SimpleNewWords d;
    score_vec_t scores = d.discover(sentences);
    std::sort(scores.begin(), scores.end(), mycomp);

    // print topk list
    for (int i = 0; i < 50; ++i)
        std::wcout << "=";
    std::wcout << std::endl;
    print_results(scores, 2);
    std::wcout << std::endl;
    for (int i = 0; i < 50; ++i)
        std::wcout << "=";
    std::wcout << std::endl;
    print_results(scores, 3);
    std::wcout << std::endl;
    for (int i = 0; i < 50; ++i)
        std::wcout << "=";
    std::wcout << std::endl;
    print_results(scores, 4);
    std::wcout << std::endl;

    // output
    std::wofstream outfile(OUTPUT_FILE, std::ios::out);
    outfile.imbue(std::locale("en_US.utf8"));
    for (auto kv: scores) {
        outfile << kv.first << " ";
        outfile << kv.second.count << " ";
        outfile << kv.second.solidity << " ";
        outfile << kv.second.entropy << " " << std::endl;
    }
    outfile.close();

    return 0;
}