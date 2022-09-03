#include "Parser.hpp"
#include "Server.hpp"
#include "ServerParameters.hpp"
#include <fstream>
#include <sstream>
#include <unistd.h>

ft::Parser::Parser() : _validConfigParams() {

}

ft::Parser::Parser(const char *config) : config_(config) {

}

ft::Parser::~Parser() {

}

int ft::Parser::isWord(std::string word, std::string line) {
	size_t length = 0;
	size_t pos = line.find(word);

	if (pos != std::string::npos) {
		if (pos > 0) {
			for (size_t i = pos; line[i]; i--) {
				if (line[i - 1] == ' ' || line[i - 1] == '\t' || line[i - 1] == '\n')
					break ;
				length++;
			}
		}
		for (size_t i = pos; line[i]; i++) {
			if (line[i] == ' ' || line[i] == '\t' || line[i] == '\n')
				break ;
			length++;
		}
		if (length != word.size() || word != line.substr(pos, length))
			return 1;
	}
	else
		return 1;
	return 0;
}

int ft::Parser::isValidBrackets(std::vector<std::string> file, size_t *line) {
	size_t openBracket = 0;
	size_t closeBracket = 0;

	size_t pos = file[*line].find("server") + strlen("server");
	while(file[*line].size()) {
		while(file[*line][pos]) {
			if (file[*line][pos] == '{')
				openBracket++;
			else if (file[*line][pos] == '}')
				closeBracket++;
			if (!openBracket && file[*line][pos] != '\n' && file[*line][pos] != ' ' && file[*line][pos] != '\t')
				return 1;
			++pos;
		}
		pos = 0;
		(*line)++;
		if (!isWord("server", file[*line])) {
			(*line)--;
			break;
		}
	}
	if (openBracket != closeBracket)
		return 1;
	return 0;
}

std::vector<std::string> ft::Parser::splitByWhitespaces(std::string key, std::string line) {
	std::vector<std::string> value;

    size_t pos = (line.find(key) + key.size());
	while (line[pos] == ' ' || line[pos] == '\t')
		pos++;

	size_t i = pos;
	while (i < line.size()) {
		pos = i;
		if (line[i] == ' ' || line[i] == '\t') {
			while (line[i] == ' ' || line[i] == '\t') {
				++pos;
				++i;
			}
		}
		else {
			while (line[i] && line[i] != ' ' && line[i] != '\t')
				i++;
			value.push_back(line.substr(pos, i - pos));
		}
	}
	return value;
}

int checkPortValue (std::string str) {
	int val = 0;

	for (size_t i = 0; i < str.size(); i++) {
		if (!isdigit(str[i]))
			throw std::invalid_argument("Parse error: Port is not digit.");
	}
	val = static_cast<int>(strtod(str.c_str(), 0));
	if (val < 0 || val > 65535)
		throw std::invalid_argument("Parse error: Port is out of range.");
	return val;
}

std::pair<int,std::string> ft::Parser::fillErrorPage(std::vector<std::string> value) {
	std::pair<int,std::string> error;

	error.first = checkPortValue(value[0]);
	error.second = value[1];

	if (value.size() > 2)
		throw std::invalid_argument("Parse error: Invalid error page information.");
	return error;
}

std::vector<std::string> ft::Parser::checkHost(std::string host) {
	size_t i = 0;
	size_t dot_count = 0;
	std::vector<std::string> hostPort;

	while(host[i]) {
		if (isdigit(host[i]))
			i++;
		else if (host[i] == '.') {
			if (dot_count > 3)
				throw std::invalid_argument("Parse error: Wrong format.");
            dot_count = 0;
			i++;
		}
		else if (host[i] == ':' && i) {
			hostPort.push_back(host.substr(0, i));
			hostPort.push_back(host.substr(i + 1, host.size()));
			return hostPort;
		}
		else
			throw std::invalid_argument("Parse error: Wrong number.");
		dot_count++;
	}
	if (!hostPort.size())
		hostPort.push_back(host.substr(0, i));
	else
		throw std::invalid_argument("Parse error: Wrong host/port.");
	return hostPort;
}

int ft::Parser::findDot(std::string value) {
	size_t pos = value.find('.');

	if (pos != std::string::npos)
		return 1;
	return 0;
}

void ft::Parser::fillHostAndPort(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (servers_[index].getHost() != 0 || servers_[index].getPort() || value.size() != 1)
		throw std::invalid_argument("Parse error: Error in host or port.");
	value = checkHost(value[0]);
	if (value.size() > 1) {
		servers_[index].setHost(value[0]);
		servers_[index].setPort(checkPortValue(value[1]));
	}
	else {
		if (!findDot(value[0]))
			servers_[index].setPort(checkPortValue(value[0]));
		else
			servers_[index].setPort(80);
	}
}

void ft::Parser::fillServerName(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (!servers_[index].getServerName().empty() || value.size() != 1)
		throw std::invalid_argument("Parse error: Error in server name.");
	servers_[index].setServerName(value[0]);
}

void ft::Parser::fillAutoIndex(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (value.size() != 1 || (value[0] != "on" && value[0] != "off"))
		throw std::invalid_argument("Parse error: Error in root AutoIndex.");
	else if (value[0] == "on")
		servers_[index].setAutoIndex(true);
}

void ft::Parser::fillServerRoot(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (!servers_[index].getRoot().empty() || value.size() != 1)
		throw std::invalid_argument("Parse error: Error in root.");
	servers_[index].setRoot(value[0]);
}

void ft::Parser::fillIndex(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (!servers_[index].getIndex().empty() || value.size() != 1)
		throw std::invalid_argument("Parse error: Error in root index page.");
	servers_[index].setIndex(value[0]);
}

void ft::Parser::fillConfig(std::string key, std::string line, ssize_t index, size_t caseKey) {
	switch (caseKey) {
		case Host_port:
			fillHostAndPort(key, line, index);
			break;
		case Server_name:
			fillServerName(key, line, index);
			break;
		case AutoIndex:
			fillAutoIndex(key, line, index);
			break;
		case Root:
			fillServerRoot(key, line, index);
			break;
		case Index_page:
			fillIndex(key, line, index);
			break;
		case Methods:
			fillRootMethods(key, line, index);
			break;
		case Client_max_body_size:
			fillRootMaxBodySize(key, line, index);
			break;
		case UploadPath:
			fillUploadPath(key, line, index);
			break;
		case Error_page:
			fillRootErrorPages(key, line, index);
			break;
	}
}

void ft::Parser::fillRootErrorPages(std::string key, std::string line, ssize_t index) {
    std::vector<std::string> value;

    std::pair <int, std::string> str;
    value = splitByWhitespaces(key, line);
    str.first = fillErrorPage(value).first;
    str.second = fillErrorPage(value).second;
    if (value.size() < 2 or !str.first or str.second.empty())
        throw std::invalid_argument("Parser error: wrong root error page format");
    servers_[index].setErrorPages(str.first, str.second);
}

void ft::Parser::fillRootMethods(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (value.size() < 1 || value.size() > 3)
		throw std::invalid_argument("Parse error: Error in root methods.");

	for (size_t i = 0; i < value.size(); ++i) {
		if (value[i] == "GET")
			servers_[index].setIsGet(true);
		else if (value[i] == "POST")
			servers_[index].setIsPost(true);
		else if (value[i] == "DELETE")
			servers_[index].setIsDelete(true);
		else
			throw std::invalid_argument("Parse error: Wrong method use GET/POST/DELETE.");
	}
}

void ft::Parser::fillRootMaxBodySize(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (servers_[index].getMaxBodySize() != 0 || value.size() != 1)
		throw std::invalid_argument("Parse error: Error in root max body size.");

	if (value[0][value[0].size() - 1] == 'M') {
		value[0] = value[0].substr(0, value[0].size() - 1);
		servers_[index].setMaxBodySize(checkPortValue(value[0]) * 1024 * 1024);
	} else if (isdigit(value[0][value[0].size() - 1])) {
		servers_[index].setMaxBodySize(checkPortValue(value[0]) * 1024);
	} else {
		throw std::invalid_argument("Parse error: Wrong value in root MaxBodySize");
	}
}

void ft::Parser::fillUploadPath(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (!servers_[index].getUploadPath().empty() || value.size() != 1)
		throw std::invalid_argument("Parse error: Error in root directory to upload.");
	servers_[index].setUploadPath(value[0]);
}

void ft::Parser::fillLocation(std::string key, std::string line, ft::Location& location, size_t caseKey) {
    std::vector<std::string> value;

    switch (caseKey) {
        case Location_name:
            fillLocationName(key, line, location);
            break;
        case Location_methods:
            fillMethods(key, line, location);
            break;
        case Location_root:
            fillLocationRoot(key, line, location);
            break;
        case Location_redirection:
            fillLocationRedirect(key, line, location);
            break;
        case Location_error_page:
            fillLocationErrorsPages(key, line, location);
            break;
        case Location_index:
            fillLocationIndex(key, line, location);
            break;
        case LocationAutoIndex:
            fillLocationAutoIndex(key, line, location);
            break;
        case LocationUploadPath:
            fillLocationUploadPath(key, line, location);
            break;
        case Bin_path_py:
            fillLocationBinPathPy(key, line, location);
            break;
        case Bin_path_sh:
            fillLocationBinPathSh(key, line, location);
            break;
    }
}


void ft::Parser::fillMethods(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (value.size() < 1 || value.size() > 3)
		throw std::invalid_argument("Parse error: Invalid location method names.");
	for (size_t i = 0; i < value.size(); i++) {
		if (value[i] == "POST")
			location.setIsPost(true);
		else if (value[i] == "GET")
			location.setIsGet(true);
		else if (value[i] == "DELETE")
			location.setIsDelete(true);
		else
			throw std::invalid_argument("Parse error: Invalid method.");
	}
}

void ft::Parser::fillLocationName(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (!location.getUrl().empty() || value.size() != 1)
		throw std::invalid_argument("Parse error: Invalid location name.");
	location.setUrl(value[0]);
	if (value[0] == "/cgi-bin/")
		location.setIsCgi(true);
}

void ft::Parser::fillLocationRoot(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (!location.getRoot().empty() || value.size() != 1)
		throw std::invalid_argument("Parse error: Invalid root location.");
	location.setRoot(value[0]);
}

void ft::Parser::fillLocationRedirect(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (location.getIsRedirect() || value.size() != 2 || location.getIsCgi())
		throw std::invalid_argument("Parse error: Invalid location redirect.");
	location.setIsRedirect(true);
	location.setIndex(value[0]);
	int code = checkPortValue(value[1]);
	if (code != 302)
		throw std::invalid_argument("Parse error: Invalid status code, you must use code 302");
	location.setRedirectionCode(code);
}

void ft::Parser::fillLocationIndex(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (!location.getIndex().empty() || value.size() != 1)
		throw std::invalid_argument("Parse error: Wrong location index.");
	location.setIndex(value[0]);
}

void ft::Parser::fillLocationAutoIndex(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (value.size() != 1 || (value[0] != "on" && value[0] != "off"))
		throw std::invalid_argument("Parse error: Invalid AutoIndex location.");
	else if (value[0] == "on")
		location.setAutoIndex(true);
}

void ft::Parser::fillLocationUploadPath(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (!location.getUploadPath().empty() || value.size() != 1)
		throw std::invalid_argument("Parse error: Invalid directory to upload.");
	location.setUploadPath(value[0]);
}

void ft::Parser::fillLocationErrorsPages(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	std::pair <int, std::string> str;
	str.first = fillErrorPage(value).first;
	str.second = fillErrorPage(value).second;
	if (value.size() < 2 || !str.first || str.second.empty())
		throw std::invalid_argument("Parse error: Invalid location error page format.");
	location.setErrorPages(str.first, str.second);
}

void ft::Parser::fillLocationBinPathPy(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (!location.getPathToBinPy().empty() || value.size() != 1 || !location.getIsCgi())
		throw std::invalid_argument("Parse error: Invalid bin location path.");
	location.setBinPathPy(value[0]);
}

void ft::Parser::fillLocationBinPathSh(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitByWhitespaces(key, line);
	if (!location.getPathToBinSh().empty() || value.size() != 1 || !location.getIsCgi())
		throw std::invalid_argument("Parse error: Invalid bin location path.");
	location.setBinPathSh(value[0]);
}

void ft::Parser::checkKeys(std::vector<std::string> config) {
    std::string newStr;

    for (std::vector<std::string>::iterator it = config.begin(); it < config.end(); ++it) {
        size_t i = 0;

        while ((*it)[i] != ' ' && (*it)[i] != '\t' && (*it)[i])
            i++;
        newStr = (*it).substr(0, i);
        if (isValidKeys(newStr))
            throw std::invalid_argument("Parse error: Incorrect key in config.");
        it++;
    }
}

void ft::Parser::serversInformation(ssize_t index, std::vector<std::string> file, size_t start, size_t end) {
	ft::ValidConfigKeys rootParams;
	std::string param;

    for (size_t j = start; j < end; ++j) {
        for (size_t i = 0; i < rootParams.serverParameters.size(); i++) {
            if (!isWord(rootParams.serverParameters[i], file[j]))
                fillConfig(rootParams.serverParameters[i], file[j], index, i); // i должен быть 0
            else if (!isWord("location", file[j])) {
                locationsInformation(file, index, &j, end);
            }
        }
    }
}

void ft::Parser::locationsInformation(std::vector<std::string> file, ssize_t index, size_t *start, size_t end) {
    ft::ValidConfigKeys locations;
    std::vector<std::string> value;
    size_t pos = 0;

    servers_[index].getLocations().push_back(Location());
    ssize_t indexLocation = servers_[index].getLocations().size() - 1;
    pos = file[*start].find("location") + strlen("location");
    for (size_t line = *start; line < end; ++line) {
        while (file[line][pos]) {
            if (file[line][pos] == '}') {
                end = line;
                break ;
            }
            pos++;
        }
        pos = 0;
    }
    while (*start < end) {
        for (size_t i = 0; i < locations.localParameters.size(); i++) {
            if (!isWord(locations.localParameters[i], file[*start])) {
                fillLocation(locations.localParameters[i], file[*start], servers_[index].getLocations()[indexLocation], i);
            }
        }
        (*start)++;
    }
}

void ft::Parser::cleanWhitespaces(std::string *line) {
	size_t i = 0;

	if ((*line)[i] == ' ' || (*line)[i] == '\t') {
		while ((*line)[i] == ' ' || (*line)[i] == '\t')
			i++;
	}
	*line = (*line).substr(i, (*line).size());
	i = (*line).size();
	if ((*line)[i - 1] == ' ' || (*line)[i - 1] == '\t' || (*line)[i - 1] == ';') {
		while ((*line)[i - 1] == ' ' || (*line)[i - 1] == '\t' || (*line)[i - 1] == ';')
			i--;
	}
	if (i < (*line).size())
		*line = (*line).substr(0, i);
}

int ft::Parser::isEmptyLine(std::string str) {
    for (int i = 0; i < str.size(); ++i)
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
            return 1;
    return 0;
}

std::vector<std::string>  ft::Parser::lineBreaks(std::string line) {
	std::vector<std::string> vec;

    for (int i = 0; i < line.size(); ++i) {
        if (line[i] != '}' && line[i] != '{')
            continue;
        std::string newString;
        std::string tmp;

        tmp = line.substr(i, line.size());
        newString = line.substr(0, i);
        cleanWhitespaces(&newString);
        if (!newString.empty())
            vec.push_back(newString);
        newString = tmp.substr(0, 1);
        vec.push_back(newString);
        newString = tmp.substr(1, tmp.size());
        cleanWhitespaces(&newString);
        if (!newString.empty())
            vec.push_back(newString);
    }

	return vec;
}

std::vector<std::string> ft::Parser::checkAndClean(std::string file_name) {
	std::ifstream file(file_name);
	std::string buffer;
	std::vector<std::string> config;

    if (file_name.compare(file_name.size() - strlen(".conf"), strlen(".conf"), std::string(".conf")) != 0 || !file.is_open())
        throw std::invalid_argument("Parse error: Wrong file type!");
	else {
		while (!file.eof()) {
			getline(file, buffer);
			if (buffer[0] == '#' || buffer.empty())
				continue;
			else if (buffer.find('#')) {
				buffer = buffer.substr(0, buffer.find('#'));
				if (!isEmptyLine(buffer))
					continue;
			}
            std::vector<std::string> tmp = lineBreaks(buffer);
			if (!tmp.empty()) {
				for (size_t i = 0; i < tmp.size(); i++) {
					buffer = tmp[i];
					tmp[i].clear();
					config.push_back(buffer);
				}
			}
			else {
                cleanWhitespaces(&buffer);
				config.push_back(buffer);
			}
			buffer.clear();
		}
	}
	config.push_back("");
	file.close();
	return config;
}

int ft::Parser::isValidKeys(std::string config) {
	if (_validConfigParams.serverKeys.find(config)->first == config)
		return 0;
	return 1;
}

void ft::Parser::parse() {
    std::vector<std::string> config;
    size_t start = 0;
    size_t end = 0;

    config = checkAndClean(config_);
    checkKeys(config);

    for (size_t line_index = 0; line_index < config.size(); ++line_index) {
        if (!isWord("server", config[line_index])) {
            start = line_index;
            if (isValidBrackets(config, &line_index)) {
                throw std::invalid_argument("Parse error: invalid brackets.");
            }
            end = line_index;
            servers_.push_back(Server());
            serversInformation(servers_.size() - 1, config, start, end);
        }
        else {
            throw std::invalid_argument("Parse error: Server not found.");
        }
    }

    config.clear();
    for (size_t i = 0; i < servers_.size(); ++i) {
        size_t j;
        for (j = 0; j < ports_.size(); ++j) {
            if (servers_[i].getPort() == ports_[j].getPort() && servers_[i].getHost() == ports_[j].getHost()) {
                break;
            }
        }
        if (j == ports_.size()) {
            ports_.push_back(Port());
            ports_[ports_.size() - 1].init(servers_[i].getPort(), servers_[i].getHost());
        }
    }
    for (size_t i = 0; i < servers_.size(); ++i) {
        size_t j;
        if (!servers_[i].getPort() || servers_[i].getRoot().size() < 2) {
            throw std::invalid_argument("Parse error: Check servers.");
        }
        for (j = 0; j < servers_[i].getLocations().size(); ++j) {
            if (servers_[i].getLocations()[j].getIsCgi() && servers_[i].getLocations()[j].getIsRedirect()) {
                throw std::invalid_argument("Parse error: Check locations.");
            }
            if (!servers_[i].getLocations()[j].getIsCgi() && !servers_[i].getLocations()[j].getIsRedirect()) {
                servers_[i].getLocations()[j].setIsFolder(true);
            }
        }
    }
}
