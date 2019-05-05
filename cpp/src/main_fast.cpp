#include "fastNewWords.h"

#include "myutils.h"
#include <codecvt>
#include <clocale>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace fastnewwords;
using namespace std;
//using namespace myutils;

bool mycomp(score_pair_t& s1, score_pair_t& s2) {
    return s1.second.count > s2.second.count;
}

void print_results(score_vec_t& scores, const int n, const int topk=50) {
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

    // set args
    const std::string INPUT_FILE = argv[1];
    const std::string OUTPUT_FILE = argv[2];

    // // get inputs
    std::ifstream infile(INPUT_FILE, std::ios_base::binary);

    FastNewWords d;
    score_vec_t scores = d.discover(infile);
    std::sort(scores.begin(), scores.end(), mycomp);
    infile.close();

    // print topk list
    for (int i = 0; i < 50; ++i)
        std::cout << "=";
    std::cout << std::endl;
    print_results(scores, 2);
    std::cout << std::endl;
    for (int i = 0; i < 50; ++i)
        std::cout << "=";
    std::cout << std::endl;
    print_results(scores, 3);
    std::cout << std::endl;
    for (int i = 0; i < 50; ++i)
        std::cout << "=";
    std::cout << std::endl;
    print_results(scores, 4);
    std::cout << std::endl;

    // output
    std::ofstream outfile(OUTPUT_FILE, std::ios::out);
    for (auto kv: scores) {
        outfile << kv.first << " ";
        outfile << kv.second.count << " ";
        outfile << kv.second.solidity << " ";
        outfile << kv.second.entropy << " " << std::endl;
    }
    outfile.close();

    return 0;
}