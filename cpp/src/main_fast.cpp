#include "fastNewWords.h"
#include "myutils.h"
#include "cxxopts.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace fastnewwords;
using namespace std;


cxxopts::ParseResult
parse_args(int argc, char* argv[])
{
  try
  {
    cxxopts::Options options(argv[0], " - example command line options");
    options
      .positional_help("[optional args]")
      .show_positional_help();

    bool apple = false;

    options
      .allow_unrecognised_options()
      .add_options()
      ("m,map_type", "Mapping type, hash or trie", cxxopts::value<std::string>()->default_value("hash"), "N")
      ("g,max_gram", "Max gram length", cxxopts::value<size_t>()->default_value("4"), "N")
      ("c,min_count", "Minimum count", cxxopts::value<size_t>()->default_value("5"), "N")
      ("s,base_solidity", "Minimum solidity of unigram", cxxopts::value<float>()->default_value("5.0"), "N")
      ("e,min_entropy", "Minimum entropy", cxxopts::value<float>()->default_value("2.0"), "N")
      ("sort", "Sort the results")
    #ifdef CXXOPTS_USE_UNICODE
      ("unicode", u8"A help option with non-ascii: à. Here the size of the"
        " string should be correct")
    #endif
    ;

    auto result = options.parse(argc, argv);
    return result;

  } catch (const cxxopts::OptionException& e)
  {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }
}

bool mycomp(const score_pair_t& s1, const score_pair_t& s2) {
    return s1.second.count > s2.second.count;
}

int main(int argc, char** argv) {
    auto result = parse_args(argc, argv);
    auto arguments = result.arguments();
    FastNewWords d(
        result["map_type"].as<std::string>(), 
        result["max_gram"].as<size_t>(),
        result["min_count"].as<size_t>(),
        result["base_solidity"].as<float>(),
        result["min_entropy"].as<float>()
        );

    score_list_t scores = d.discover(std::cin);
    if (result["sort"].count() > 0)
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