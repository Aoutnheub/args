#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <regex>
#include <algorithm>
#include <cstring>
#include <cerrno>

namespace args {
    class invalid_argument : public std::exception {
        private:
            std::string arg;
            std::string error;

        public:
            invalid_argument(const std::string &_arg);
            virtual const char* what() const throw();
            const char* which();
    };

    class invalid_value : public std::exception {
        private:
            std::string arg;
            std::string value;
            std::string error;

        public:
            invalid_value(const std::string &_arg, const std::string &_value);
            virtual const char* what() const throw();
            const char* which();
            const char* with();
    };

    class missing_value : public std::exception {
        private:
            std::string arg;
            std::string error;

        public:
            missing_value(const std::string &_arg);
            virtual const char* what() const throw();
            const char* which();
    };

    class duplicate_argument : public std::exception {
        private:
            std::string arg;
            std::string error;

        public:
            duplicate_argument(const std::string &_arg);
            virtual const char* what() const throw();
            const char* which();
    };
}

class ArgFlag {
    public:
        std::string help;

        ArgFlag(const std::string &_help);
};

class ArgOption {
    public:
        std::string help;
        std::string defaults_to;
        std::vector<std::string> allowed;

        ArgOption(const std::string &_help = "", const std::string &_defaults_to = "", const std::vector<std::string> &_allowed = {});
};

class ArgResults {
    public:
        std::unordered_map<std::string, bool> flag;
        std::unordered_map<std::string, std::string> option;
        std::vector<std::string> positional;
        std::string command;

        ArgResults();
};

class ArgParser {
    private:
        std::unordered_map<std::string, ArgFlag*> flags;
        std::unordered_map<std::string, std::string> flags_abbr;
        std::unordered_map<std::string, ArgOption*> options;
        std::unordered_map<std::string, std::string> options_abbr;
        std::unordered_map<std::string, std::string> commands;
        std::vector<std::string> positional;
        std::string name;
        std::string description;

        bool searchOption(const std::vector<std::string> &_a, const std::string &_sa, const std::size_t &_i, ArgResults &_r);
        bool searchFlag(const std::string &_sa, ArgResults &_r);
        std::string findFlagAbbr(const std::string &_name);
        std::string findOptionAbbr(const std::string &_name);

    public:
        ArgParser(const std::string &_name = "", const std::string &_description = "");
        ~ArgParser();

        void addFlag(const std::string &_name, const std::string &_help = "", const char &_abbr = '\0');
        void addOption(const std::string &_name, const std::string &_help = "", const char &_abbr = '\0', const std::string &_defaults_to = "", const std::vector<std::string> &_allowed = {});
        void addCommand(const std::string &_name, const std::string &_help = "");
        std::string help();
        ArgResults parse(const std::vector<std::string> &_args) {
            ArgResults results;

            for(auto i = flags.begin(); i != flags.end(); ++i) {
                results.flag.insert(results.flag.end(), {i->first, false});
            }
            for(auto i = options.begin(); i != options.end(); ++i) {
                results.option.insert(results.option.end(), {i->first, i->second->defaults_to});
            }

            std::size_t i = 0;
            while(i < _args.size()) {
                if(!_args[i].empty()) {
                    if(std::regex_match(_args[i], std::regex("^-[^-].*$"))) {
                        if(_args[i].length() > 2) {
                            // multiple flags
                            for(std::size_t j = 1; j < _args[i].length(); ++j) {
                                std::string f;
                                f += _args[i][j];
                                if(!searchFlag(f, results)) {
                                    throw args::invalid_argument(f);
                                }
                            }
                            ++i;
                        }else {
                            std::string stripped_arg = _args[i].substr(1);
                            if(!searchOption(_args, stripped_arg, i, results)) {
                                if(!searchFlag(stripped_arg, results)) {
                                    throw args::invalid_argument(stripped_arg);
                                }else {
                                    ++i;
                                }
                            }else {
                                i += 2;
                            }
                        }
                    }else if(std::regex_match(_args[i], std::regex("^--[^-].*$"))) {
                        std::string stripped_arg = _args[i].substr(2);
                        if(!searchOption(_args, stripped_arg, i, results)) {
                            if(!searchFlag(stripped_arg, results)) {
                                throw args::invalid_argument(stripped_arg);
                            }else {
                                ++i;
                            }
                        }else {
                            i += 2;
                        }
                    }else if(i == 0 && !commands.empty()) {
                        if(commands.find(_args[i]) != commands.end()) {
                            results.command = _args[i];
                            ++i;
                        }else {
                            results.positional.push_back(_args[i]);
                            ++i;
                        }
                    }
                }else {
                    ++i;
                }
            }

            return results;
        }

        template<class T, class = typename std::enable_if<std::is_same<T, char>::value>::type, std::size_t S, std::size_t S2>
        ArgResults parse(const T (&_args)[S][S2]) {
            std::vector<std::string> v;
            for(std::size_t i = 0; i < S; ++i) {
                std::string s = _args[i];
                v.push_back(s);
            }

            return parse(v);
        }

        template<class T, class = typename std::enable_if<std::is_same<T, char*>::value>::type, std::size_t S>
        ArgResults parse(const T (&_args)[S]) {
            std::vector<std::string> v;
            for(std::size_t i = 0; i < S; ++i) {
                std::string s = _args[i];
                v.push_back(s);
            }

            return parse(v);
        }

        template<class T, class = typename std::enable_if<std::is_same<T, char**>::value>::type>
        ArgResults parse(const T (&_args), const std::size_t S) {
            std::vector<std::string> v;
            for(std::size_t i = 0; i < S; ++i) {
                std::string s = _args[i];
                v.push_back(s);
            }

            return parse(v);
        }

};
