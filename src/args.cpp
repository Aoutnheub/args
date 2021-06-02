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

bool ArgParser::validateOption(const std::vector<std::string> &_a, std::string _sa, std::size_t &_i, ArgResults &_r) {
    std::string val;
    std::size_t equals = _sa.find("=");
    if(_sa.length() < 2) {
        if(_i+1 < _a.size()) {
            val = _a[_i+1];
        }else {
            val = "";
        }
    }else {
        if(equals != std::string::npos) {
            if(equals != _sa.size()-1) {
                val = _sa.substr(equals+1);
                _sa = _sa.substr(0, equals);
                --_i;
            }else {
                throw args::missing_value(_sa.substr(0, equals));
            }
        }else {
            if(options_abbr.find(_sa.substr(0, 1)) != options_abbr.end()) {
                if(_i+1 < _a.size()) {
                    val = _a[_i+1];
                }
                //--_i;
            }else {
                if(_i+1 < _a.size()) {
                    val = _a[_i+1];
                }else {
                    val = "";
                }
            }
        }
    }

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
        if(val.empty() || val[0] == '-') {
            throw args::missing_value(_sa);
        }else {
            ArgOption *valid_option = found_option->second;
            if(!valid_option->allowed.empty()) {
                bool valid_value = false;
                for(auto o : valid_option->allowed) {
                    if(o == val) {
                        valid_value = true;
                        break;
                    }
                }
                if(valid_value) {
                    _r.option[found_option->first] = val;
                }else {
                    throw args::invalid_value(_sa, val);
                }
            }else {
                _r.option[found_option->first] = val;
            }
        }

        return true;
    }else {
        return false;
    }
}

bool ArgParser::validateFlag(const std::string &_sa, ArgResults &_r) {
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

void ArgParser::splitDesc(std::string &_help, std::string &_desc) {
    std::size_t desc_length = _desc.length();
    std::size_t last = 0;
    int line_length = max_line_length-8;
    std::string indent = "        ";
    while(desc_length > line_length) {
        std::size_t token;
        if(last+line_length >= last+desc_length) {
            _help += indent + _desc.substr(last) + '\n';
            break;
        }else {
            token = _desc.rfind(" ", last+line_length);
        }
        if(token != std::string::npos && token > last) {
            _help += indent + _desc.substr(last, token-last) + '\n';
            desc_length -= token-last-1;
            last = token+1;
        }else {
            _help += indent + _desc.substr(last) + '\n';
            break;
        }
    }
    _help += indent + _desc.substr(last) + '\n';
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
        helpful += " - ";
        if(description.length() > max_line_length-name.length()-3) {
            std::size_t desc_length = description.length();
            std::size_t last = 0;
            int line_length = max_line_length-name.length()-3;
            std::string indent;
            for(std::size_t i = 0; i < name.length()+3; ++i) {
                indent += " ";
            }
            std::size_t tk = description.rfind(" ", last+line_length);
            if(tk != std::string::npos) {
                helpful += description.substr(last, tk-last) + '\n';
                desc_length -= tk-last-1;
                last = tk+1;
            }
            while(desc_length > line_length) {
                std::size_t token;
                if(last+line_length >= last+desc_length) {
                    helpful += indent + description.substr(last) + '\n';
                    break;
                }else {
                    token = description.rfind(" ", last+line_length);
                }
                if(token != std::string::npos && token > last) {
                    helpful += indent + description.substr(last, token-last) + '\n';
                    desc_length -= token-last-1;
                    last = token+1;
                }else {
                    helpful += indent + description.substr(last) + '\n';
                    break;
                }
            }
            helpful += indent + description.substr(last) + '\n';
        }else {
            helpful += description + '\n';
        }
    }
    helpful += "\n";

    if(!commands.empty()) {
        helpful += "COMMANDS\n\n";
        for(auto i = commands.begin(); i != commands.end(); ++i) {
            helpful += "    "+i->first+'\n';
            std::string indent = "        ";
            if(i->second.length() > max_line_length-8) {
                splitDesc(helpful, i->second);
            }else {
                helpful += indent + i->second + '\n';
            }
            helpful += '\n';
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
            // helpful += "\n        "+i->second->help+"\n\n";
            helpful += '\n';
            std::string indent = "        ";
            if(i->second->help.length() > max_line_length-8) {
                splitDesc(helpful, i->second->help);
            }else {
                helpful += indent + i->second->help + '\n';
            }
            helpful += '\n';
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
            helpful += '\n';
            std::string indent = "        ";
            if(i->second->help.length() > max_line_length-8) {
                splitDesc(helpful, i->second->help);
            }else {
                helpful += indent + i->second->help + '\n';
            }
            helpful += '\n';
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
                    std::string stripped_arg = _args[i].substr(1);
                    if(!validateOption(_args, stripped_arg, i, results)) {
                        for(std::size_t j = 0; j < stripped_arg.length(); ++j) {
                            std::string f;
                            f += stripped_arg[j];
                            if(!validateFlag(f, results)) {
                                throw args::invalid_argument(f);
                            }
                        }
                        ++i;
                    }else {
                        i += 2;
                    }
                }else {
                    std::string stripped_arg = _args[i].substr(1);
                    if(!validateOption(_args, stripped_arg, i, results)) {
                        if(!validateFlag(stripped_arg, results)) {
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
                if(!validateOption(_args, stripped_arg, i, results)) {
                    if(!validateFlag(stripped_arg, results)) {
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
            }else {
                results.positional.push_back(_args[i]);
                ++i;
            }
        }else {
            ++i;
        }
    }

    return results;
}
