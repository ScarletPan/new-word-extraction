#include "fastNewWords.h"
#include "myutils.h"
#include <algorithm>
#include <clocale>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace fastnewwords;
using namespace std;
//using namespace myutils;

bool mycomp(const score_pair_t& s1, const score_pair_t& s2) {
    return s1.second.count > s2.second.count;
}

void print_results(score_list_t& scores, const int n, const int topk=50) {
    int cnt = 0;
    for (int i = 0; i < scores.size(); ++i) {
        auto kv = scores[i];
        if (myutils::size_of_utf8(kv.first) == n) {
            std::cout << kv.first << " ";
            // std::cout << kv.second.count << " ";
            // std::cout << kv.second.solidity << std::setprecision(2) << " ";
            // std::cout << kv.second.entropy << std::setprecision(2) << " " << std::endl;
            if (++cnt == topk)
                return;
        }
    }
    std::wcout << std::endl;
}


int main(int argc, char** argv) {
    FastNewWords d;
    score_list_t scores = d.discover(std::cin);
    std::sort(scores.begin(), scores.end(), mycomp);
    // output
    for (auto kv: scores) {
        std::cout << kv.first << " ";
        std::cout << kv.second.count << " ";
        std::cout << kv.second.solidity << " ";
        std::cout << kv.second.entropy << " " << std::endl;
    }

    return 0;
}