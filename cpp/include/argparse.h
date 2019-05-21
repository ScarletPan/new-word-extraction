// Copyright (c) 2019-present, Tencent, Inc.
// All rights reserved.
// 
// Author: Haojie Pan
// Email: jasonhjpan@tencent.com
//

#ifndef NEWWORDS_ARGPARSE_H_
#define NEWWORDS_ARGPARSE_H_

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <map>

namespace argparse {

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


class Option {
public:
    std::string type;
    std::string value;
    std::string help;

    Option() {};
    Option(const std::string& type, const std::string& value, const std::string& help) {
        this->type = type;
        this->value = value;
        this->help = help;
    }
};

class ArgumentParser {
public:
    ArgumentParser() {};
    ArgumentParser(const std::string& desc) {
        this->description = desc;
    }

    void add_argument(const std::string& optind, const std::string& type="string", 
                      const std::string& dvalue="", const std::string& help="") {
        this->options.insert({optind, Option(type, dvalue, help)});
    }

    bool parse_args(int argc, char** argv) {
        if (cmdOptionExists(argv, argv+argc, "-h") || cmdOptionExists(argv, argv+argc, "--help")) {
            reportHelpInfo();
            return false;
        }
        for (auto& kv: options) {
            if (kv.second.type == "bool") {
                kv.second.value = "1";
            } else if (cmdOptionExists(argv, argv + argc, kv.first)) {
                kv.second.value = getCmdOption(argv, argv + argc, kv.first);
            }
        }
        return true;
    }

    void get(const std::string& optname, std::string& ret) {
        ret = options["--" + optname].value;
    }

    void get(const std::string& optname, int& ret) {
        ret = std::atoi(options["--" + optname].value.c_str());
    }

    void get(const std::string& optname, float& ret) {
        ret = std::atof(options["--" + optname].value.c_str());
    }

    void get(const std::string& optname, bool& ret) {
        if (options["--" + optname].value == "0")
            ret = false;
        else
            ret = true;
    }

private:
    std::string description;
    std::map<std::string, Option> options;

    void reportHelpInfo() {
        std::cerr << " * " + this->description + "\n Usage:\n";
        for (auto& kv: options) {
            Option& opt = kv.second;
            std::cerr << "   " << std::left 
                      << std::setw(20) << kv.first + " arg " 
                      << opt.help << std::endl;
        }
        std::cerr << "   " << std::left 
                  << std::setw(20) << "-h, --help"
                  << "Print help\n\n";
    }

};

}

#endif // NEWWORDS_ARGPARSE_H_