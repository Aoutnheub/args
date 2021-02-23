#include "../headers/args.h"

// Exceptions
args::invalid_argument::invalid_argument(const std::string &_arg) : arg(_arg) {
    error = "argument \""+arg+"\" does not exist";
}

const char* args::invalid_argument::what() const throw() {
    return error.c_str();
}

const char* args::invalid_argument::which() {
    return arg.c_str();
}

args::invalid_value::invalid_value(const std::string &_arg, const std::string &_value) : arg(_arg), value(_value) {
    error = "no value named \""+value+"\" for argument \""+arg+"\"";
}

const char* args::invalid_value::what() const throw() {
    return error.c_str();
}

const char* args::invalid_value::which() {
    return arg.c_str();
}

const char* args::invalid_value::with() {
    return value.c_str();
}

args::missing_value::missing_value(const std::string &_arg) : arg(_arg) {
    error = "no value provided for argument \""+arg+"\"";
}

const char* args::missing_value::what() const throw() {
    return error.c_str();
}

const char* args::missing_value::which() {
    return arg.c_str();
}

args::duplicate_argument::duplicate_argument(const std::string &_arg) : arg(_arg) {
    error = "argument \""+arg+"\" already exists";
}

const char* args::duplicate_argument::what() const throw() {
    return error.c_str();
}

const char* args::duplicate_argument::which() {
    return arg.c_str();
}

// Flag
ArgFlag::ArgFlag(const std::string &_help) : help(_help) {}

// Option
ArgOption::ArgOption(
    const std::string &_help,
    const std::string &_defaults_to,
    const std::vector<std::string> &_allowed
) : help(_help), defaults_to(_defaults_to), allowed(_allowed) {}

// Results
ArgResults::ArgResults() {}

// Parser
ArgParser::ArgParser(const std::string &_name, const std::string &_description) : name(_name), description(_description) {}

ArgParser::~ArgParser() {
    for(auto i = flags.begin(); i != flags.end(); ++i) {
        delete i->second;
    }
    for(auto i = options.begin(); i != options.end(); ++i) {
        delete i->second;
    }
}

bool ArgParser::searchOption(const std::vector<std::string> &_a, const std::string &_sa, const std::size_t &_i, ArgResults &_r) {
    auto found_option = options.find(_sa);
    bool found = false;
    if(found_option == options.end()) {
        auto found_abbr = options_abbr.find(_sa);
        if(found_abbr != options_abbr.end()) {
            found_option = options.find(found_abbr->second);
            found = true;
        }
    }else {
        found = true;
    }

    if(found) {
        if(_i+1 < _a.size()) {
            if(_a[_i+1][0] != '-') {
                ArgOption *valid_option = found_option->second;
                if(!valid_option->allowed.empty()) {
                    bool valid_value = false;
                    for(auto o : valid_option->allowed) {
                        if(o == _a[_i+1]) {
                            valid_value = true;
                            break;
                        }
                    }
                    if(valid_value) {
                        _r.option[found_option->first] = _a[_i+1];
                    }else {
                        throw args::invalid_value(_sa, _a[_i+1]);
                    }
                }else {
                    _r.option[found_option->first] = _a[_i+1];
                }
            }else {
                throw args::missing_value(_sa);
            }
        }else {
            throw args::missing_value(_sa);
        }

        return true;
    }else {
        return false;
    }
}

bool ArgParser::searchFlag(const std::string &_sa, ArgResults &_r) {
    auto found_flag = flags.find(_sa);
    bool found = false;
    if(found_flag == flags.end()) {
        auto found_abbr = flags_abbr.find(_sa);
        if(found_abbr != flags_abbr.end()) {
            found_flag = flags.find(found_abbr->second);
            found = true;
        }
    }else {
        found = true;
    }

    if(found) {
        _r.flag[found_flag->first] = true;
        return true;
    }else {
        return false;
    }
}

std::string ArgParser::findFlagAbbr(const std::string &_name) {
    for(auto i = flags_abbr.begin(); i != flags_abbr.end(); ++i) {
        if(i->second == _name) {
            return std::string(i->first);
        }
    }
    return "";
}

std::string ArgParser::findOptionAbbr(const std::string &_name) {
    for(auto i = options_abbr.begin(); i != options_abbr.end(); ++i) {
        if(i->second == _name) {
            return std::string(i->first);
        }
    }
    return "";
}

void ArgParser::addFlag(const std::string &_name, const std::string &_help, const char &_abbr) {
    if(flags.find(_name) == flags.end() && options.find(_name) == options.end()) {
        ArgFlag *argf = new ArgFlag(_help);
        flags.insert({_name, argf});
        if(_abbr != '\0') {
            std::string temp;
            temp += _abbr;
            flags_abbr.insert({temp, _name});
        }
    }else {
        throw args::duplicate_argument(_name);
    }
}

void ArgParser::addOption(
    const std::string &_name,
    const std::string &_help,
    const char &_abbr,
    const std::string &_defaults_to,
    const std::vector<std::string> &_allowed
) {
    if(options.find(_name) == options.end() && flags.find(_name) == flags.end()) {
        ArgOption *argo = new ArgOption(_help, _defaults_to, _allowed);
        options.insert({_name, argo});
        if(_abbr != '\0') {
            std::string temp;
            temp += _abbr;
            options_abbr.insert({temp, _name});
        }
    }else {
        throw args::duplicate_argument(_name);
    }
}

void ArgParser::addCommand(const std::string &_name, const std::string &_help) {
    if(commands.find(_name) == commands.end()) {
        commands.insert({_name, _help});
    }else {
        throw args::duplicate_argument(_name);
    }
}

std::string ArgParser::help() {
    std::string helpful;
    if(!name.empty()) {
        helpful += name;
    }
    if(!description.empty()) {
        helpful += " - "+description;
    }
    helpful += "\n\n";
    if(!commands.empty()) {
        helpful += "COMMANDS\n\n";
        for(auto i = commands.begin(); i != commands.end(); ++i) {
            helpful += "    "+i->first+'\n';
            helpful += "        "+i->second+"\n\n";
        }
    }
    if(!flags.empty()) {
        helpful += "FLAGS\n\n";
        for(auto i = flags.begin(); i != flags.end(); ++i) {
            helpful += "    --"+i->first;
            std::string abbr = findFlagAbbr(i->first);
            if(!abbr.empty()) {
                helpful += ", -"+abbr;
            }
            helpful += "\n        "+i->second->help+"\n\n";
        }
    }
    if(!options.empty()) {
        helpful += "OPTIONS\n\n";
        for(auto i = options.begin(); i != options.end(); ++i) {
            helpful += "    --"+i->first;
            std::string abbr = findOptionAbbr(i->first);
            if(!abbr.empty()) {
                helpful += ", -"+abbr;
            }
            if(!i->second->allowed.empty()) {
                helpful += " ";
                for(std::size_t ii = 0; ii < i->second->allowed.size(); ++ii) {
                    if(ii != i->second->allowed.size()-1) {
                        helpful += i->second->allowed[ii]+"|";
                    }else {
                        helpful += i->second->allowed[ii];
                    }
                }
            }
            helpful += "\n        "+i->second->help+"\n\n";
        }
    }

    return helpful;
}

ArgResults ArgParser::parse(const std::vector<std::string> &_args) {
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
