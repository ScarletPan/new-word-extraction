#include "infoEnt.h"
#include <codecvt>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace fastnewwords;


const std::string INPUT_FILE = "/Users/myscarlet/Job/Intern/Wechat-2019/projects/new-word-extraction/"
                               "data/books/tianlongbabu_jinyong.txt";


bool mycomp(score_pair_t& s1, score_pair_t& s2) {
    return s1.second.count > s2.second.count;
}

int main() {
    const std::locale utf8(std::locale(), new std::codecvt_utf8<wchar_t>);
    std::wcout.imbue(utf8);
    std::wifstream infile(INPUT_FILE, std::ios::in);
    infile.imbue(utf8);


    std::vector<std::wstring> sentences;
    for (std::wstring line; infile >> line;) {
        sentences.push_back(line);
    }
    FastNewWords d;
    score_vec_t scores = d.discover(sentences);
    std::sort(scores.begin(), scores.end(), mycomp);
    for (int i = 0; i < 50; ++i) {
        auto kv = scores[i];
        std::wcout << kv.first << " ";
        std::wcout << kv.second.count << " ";
        std::wcout << kv.second.solidity << std::setprecision(2) << " ";
        std::wcout << kv.second.entropy << std::setprecision(2) << " " << std::endl;
    }
    return 0;
}