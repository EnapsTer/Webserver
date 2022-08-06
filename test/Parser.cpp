#include "Parser.hpp"
#include "Server.hpp"
#include "ServerParameters.hpp"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <unistd.h>

ft::Parser::Parser() : _validConfigParams() {

}

ft::Parser::Parser(const char *config) : _config(config) {

}

ft::Parser::~Parser() {

}

void ft::Parser::parse() {
	std::vector<std::string> config;
	size_t start = 0;
	size_t line = 0;
	size_t end = 0;

	config = checkAndClean(_config);
	checkAllKeys(config);
	while (line < config.size()) {
		if (!easyFind("server", config[line])) {
			start = line;
			if (checkBrackets(config, &line) < 0) {
				throw std::invalid_argument("Parser error: brackets");
			}
			end = line;
			_servers.push_back(Server());
			serversInfo(_servers.size() - 1, config, start, end);
		}
		else {
			throw std::invalid_argument("Parser error: server not found");
		}
		line++;
	}
	config.clear();
	for (size_t i = 0; i < _servers.size(); ++i) {
		size_t j;
		for (j = 0; j < _ports.size(); ++j) {
			if (_servers[i].getPort() == _ports[j].getPort() and _servers[i].getHost() == _ports[j].getHost()) {
				break;
			}
		}
		if (j == _ports.size()) {
			_ports.push_back(Port());
			_ports[_ports.size() - 1].init(_servers[i].getPort(), _servers[i].getHost());
		}
	}
	for (size_t i = 0; i < _servers.size(); ++i) {
		size_t j;
		if (!_servers[i].getPort() or _servers[i].getRoot().size() < 2) {
			throw std::invalid_argument("Parser error: final check servers");
		}
		for (j = 0; j < _servers[i].getLocations().size(); ++j) {
			if (_servers[i].getLocations()[j].getIsCgi() and _servers[i].getLocations()[j].getIsRedirect()) {
				throw std::invalid_argument("Parser error: final check locations");
			}
			if (not _servers[i].getLocations()[j].getIsCgi() and not _servers[i].getLocations()[j].getIsRedirect()) {
				_servers[i].getLocations()[j].setIsFolder(true);
			}
		}
	}
}

const std::vector<ft::Server> &ft::Parser::getServers() const {
	return _servers;
}

int ft::Parser::easyFind(std::string word, std::string line) {
	size_t length = 0;
	size_t pos = -1;

	pos = line.find(word);
	if (pos != std::string::npos) {
		if (pos > 0) {
			for (size_t i = pos; line[i]; i--) {
				if (line[i - 1] == ' ' or line[i - 1] == '\t' or line[i - 1] == '\n')
					break ;
				length++;
			}
		}
		for (size_t i = pos; line[i]; i++) {
			if (line[i] == ' ' or line[i] == '\t' or line[i] == '\n')
				break ;
			length++;
		}
		if (length != word.size() or word != line.substr(pos, length))
			return -1;
	}
	else
		return -1;
	return 0;
}

int ft::Parser::checkBrackets(std::vector<std::string> file, size_t *line) {
	size_t openBracket = 0;
	size_t closeBracket = 0;
	size_t pos;

	pos = file[*line].find("server") + strlen("server");
	while(file[*line].size()) {
		while(file[*line][pos]) {
			if (file[*line][pos] == '{')
				openBracket++;
			else if (file[*line][pos] == '}')
				closeBracket++;
			if (!openBracket and file[*line][pos] != ' ' and
				file[*line][pos] != '\t' and file[*line][pos] != '\n')
				return -1;
//						if (openBracket == 1 and closeBracket == openBracket)
//							return -1;
			pos++;
		}
		pos = 0;
		(*line)++;
		if (!easyFind("server", file[*line])) {
			(*line)--;
			break ;
		}
	}
	if (openBracket != closeBracket)
		return -1;
	return 0;
}

std::vector<std::string> ft::Parser::splitString(std::string key, std::string line) {
	std::vector<std::string> value;
	size_t pos = 0;
	size_t i = 0;

	pos = (line.find(key) + key.size());
	while (line[pos] == ' ' or line[pos] == '\t') {
		pos++;
	}
	i = pos;
	while (i < line.size()) {
		pos = i;
		if (line[i] == ' ' or line[i] == '\t') {
			while (line[i] == ' ' or line[i] == '\t') {
				i++;
				pos++;
			}
		}
		else {
			while (line[i] and line[i] != ' ' and line[i] != '\t')
				i++;
			value.push_back(line.substr(pos, i - pos));
		}
	}
	return value;
}

int checkPortVal (std::string str) {
	int val = 0;

	for (size_t i = 0; i < str.size(); i++) {
		if (!isdigit(str[i]))
			throw std::invalid_argument("Parser error: not digit");
	}
	val = static_cast<int>(strtod(str.c_str(), 0));
	if (val < 0 or val > 65535)
		throw std::invalid_argument("Parser error: number out of range");
	return val;
}

std::pair<int,std::string> ft::Parser::fillErrorPage(std::vector<std::string> value) {
	std::pair<int,std::string> error;

	error.first = checkPortVal(value[0]);
	error.second = value[1];
	if (value.size() > 2)
		throw std::invalid_argument("Parser error: invalid error page info");
	return error;
}

std::vector<std::string> ft::Parser::checkHost(std::string host) {
	size_t i = 0;
	size_t j = 0;
	size_t dot = 0;
	std::vector<std::string> hostPort;

	while(host[i]) {
		if (isdigit(host[i]))
			i++;
		else if (host[i] == '.') {
			if (j > 3)
				throw std::invalid_argument("Parser error: wrong format");
			j = 0;
			dot++;
			i++;
		}
		else if (host[i] == ':' and i) {
			hostPort.push_back(host.substr(0, i));
			hostPort.push_back(host.substr(i + 1, host.size()));
			return hostPort;
		}
		else
			throw std::invalid_argument("Parser error: wrong number");
		j++;
	}
	if (!hostPort.size())
		hostPort.push_back(host.substr(0, i));
	else
		throw std::invalid_argument("Parser error: wrong host/port");
	return hostPort;
}

int ft::Parser::whatIsIt(std::string value) {
	size_t pos = value.find(".");

	if (pos != std::string::npos)
		return 1;
	return 0;
}

void ft::Parser::fillHostPort(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (_servers[index].getHost() != 0 or _servers[index].getPort() or value.size() != 1)
		throw std::invalid_argument("Parser error: host/port error");
	value = checkHost(value[0]);
	if (value.size() > 1) {
		_servers[index].setHost(value[0]);
		_servers[index].setPort(checkPortVal(value[1]));
	}
	else {
		if (!whatIsIt(value[0]))
			_servers[index].setPort(checkPortVal(value[0]));
		else
			_servers[index].setPort(80);
	}
}

void ft::Parser::fillServerName(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (!_servers[index].getServerName().empty() or value.size() != 1)
		throw std::invalid_argument("Parser error: server name error");
	_servers[index].setServerName(value[0]);
}

void ft::Parser::fillAutoindex(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (value.size() != 1 or (value[0] != "on" and value[0] != "off"))
		throw std::invalid_argument("Parser error: root autoindex error");
	else if (value[0] == "on")
		_servers[index].setAutoIndex(true);
}

void ft::Parser::fillServerRoot(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (!_servers[index].getRoot().empty() or value.size() != 1)
		throw std::invalid_argument("Parser error: root error");
	_servers[index].setRoot(value[0]);
}

void ft::Parser::fillIndex(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (!_servers[index].getIndex().empty() or value.size() != 1)
		throw std::invalid_argument("Parser error: root index page error");
	_servers[index].setIndex(value[0]);
}

void ft::Parser::fillRootMethods(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (value.size() < 1 or value.size() > 3)
		throw std::invalid_argument("Parser error: root methods error");
	for (size_t i = 0; i < value.size(); i++) {
		if (value[i] == "GET")
			_servers[index].setIsGet(true);
		else if (value[i] == "POST")
			_servers[index].setIsPost(true);
		else if (value[i] == "DELETE")
			_servers[index].setIsDelete(true);
		else
			throw std::invalid_argument("Parser error: wrong root method");
	}
}

void ft::Parser::fillRootMaxBodySize(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (_servers[index].getMaxBodySize() != 0 or value.size() != 1)
		throw std::invalid_argument("Parser error: root max body size error");
	if (value[0][value[0].size() - 1] == 'M') {
		value[0] = value[0].substr(0, value[0].size() - 1);
		_servers[index].setMaxBodySize(checkPortVal(value[0]) * 1024 * 1024);
	} else if (isdigit(value[0][value[0].size() - 1])) {
		_servers[index].setMaxBodySize(checkPortVal(value[0]) * 1024);
	} else {
		throw std::invalid_argument("Parser error: wrong value root MaxBodySize");
	}
}

void ft::Parser::fillUploadPath(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (!_servers[index].getUploadPath().empty() or value.size() != 1)
		throw std::invalid_argument("Parser error: root directory to upload error");
	_servers[index].setUploadPath(value[0]);
}

void ft::Parser::fillRootErrorPages(std::string key, std::string line, ssize_t index) {
	std::vector<std::string> value;

	std::pair <int, std::string> str;
	value = splitString(key, line);
	str.first = fillErrorPage(value).first;
	str.second = fillErrorPage(value).second;
	if (value.size() < 2 or !str.first or str.second.empty())
		throw std::invalid_argument("Parser error: wrong root error page format");
	_servers[index].setErrorPages(str.first, str.second);
}


void ft::Parser::fillConfig(std::string key, std::string line, ssize_t index, size_t caseKey) {
	switch (caseKey) {
		case Host_port:
			fillHostPort(key, line, index);
			break;
		case Server_name:
			fillServerName(key, line, index);
			break;
		case Autoindex:
			fillAutoindex(key, line, index);
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

void ft::Parser::fillLocationName(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (!location.getUrl().empty() or value.size() != 1)
		throw std::invalid_argument("Parser error: location error");
	location.setUrl(value[0]);
	if (value[0] == "/cgi-bin/")
		location.setIsCgi(true);
}

void ft::Parser::fillLocationMethods(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (value.size() < 1 or value.size() > 3)
		throw std::invalid_argument("Parser error: location methods error");
	for (size_t i = 0; i < value.size(); i++) {
		if (value[i] == "GET")
			location.setIsGet(true);
		else if (value[i] == "POST")
			location.setIsPost(true);
		else if (value[i] == "DELETE")
			location.setIsDelete(true);
		else
			throw std::invalid_argument("Parser error: wrong location method");
	}
}

void ft::Parser::fillLocationRoot(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (!location.getRoot().empty() or value.size() != 1)
		throw std::invalid_argument("Parser error: root location error");
	location.setRoot(value[0]);
}

void ft::Parser::fillLocationIndex(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (!location.getIndex().empty() or value.size() != 1)
		throw std::invalid_argument("Parser error: wrong lcoation index page");
	location.setIndex(value[0]);
}

void ft::Parser::fillLocationRedirection(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (location.getIsRedirect() or value.size() != 2 or location.getIsCgi())
		throw std::invalid_argument("Parser error: location redirection error");
	location.setIsRedirect(true);
	location.setIndex(value[0]);
	int code = checkPortVal(value[1]);
	if (code != 302)
		throw std::invalid_argument("Parser error: wrong number, you can use only code 302");
	location.setRedirectionCode(code);
}

void ft::Parser::fillLocationAutoindex(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (value.size() != 1 or (value[0] != "on" and value[0] != "off"))
		throw std::invalid_argument("Parser error: location Autoindex error");
	else if (value[0] == "on")
		location.setAutoIndex(true);
}

void ft::Parser::fillLocationUploadPath(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (!location.getUploadPath().empty() or value.size() != 1)
		throw std::invalid_argument("Parser error: wrong location directory to upload");
	location.setUploadPath(value[0]);
}

void ft::Parser::fillLocationErrorsPages(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitString(key, line);
	std::pair <int, std::string> str;
	str.first = fillErrorPage(value).first;
	str.second = fillErrorPage(value).second;
	if (value.size() < 2 or !str.first or str.second.empty())
		throw std::invalid_argument("Parser error: wrong location error page format");
	location.setErrorPages(str.first, str.second);
}

void ft::Parser::fillLocationBinPathPy(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (!location.getBinPathPy().empty() or value.size() != 1 or !location.getIsCgi())
		throw std::invalid_argument("Parser error: wrong location bin path");
	location.setBinPathPy(value[0]);
}

void ft::Parser::fillLocationBinPathSh(std::string key, std::string line, ft::Location& location) {
	std::vector<std::string> value;

	value = splitString(key, line);
	if (!location.getBinPathSh().empty() or value.size() != 1 or !location.getIsCgi())
		throw std::invalid_argument("Parser error: wrong location bin path");
	location.setBinPathSh(value[0]);
}

void ft::Parser::fillLocation(std::string key, std::string line, ft::Location& location, size_t caseKey) {
	std::vector<std::string> value;

	switch (caseKey) {
		case Location_name:
			fillLocationName(key, line, location);
			break;
		case Location_methods:
			fillLocationMethods(key, line, location);
			break;
		case Location_root:
			fillLocationRoot(key, line, location);
			break;
		case Location_redirection:
			fillLocationRedirection(key, line, location);
			break;
		case Location_error_page:
			fillLocationErrorsPages(key, line, location);
			break;
		case Location_index:
			fillLocationIndex(key, line, location);
			break;
		case LocationAutoindex:
			fillLocationAutoindex(key, line, location);
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

void ft::Parser::locationsInfo(std::vector<std::string> file, ssize_t index, size_t *start, size_t end) {
	ft::ValidConfigKeys locations;
	std::vector<std::string> value;
	size_t line = *start;
	size_t pos = 0;

	_servers[index].getLocations().push_back(Location());
	ssize_t indexLocation = _servers[index].getLocations().size() - 1;
	pos = file[*start].find("location") + strlen("location");
	while (line < end ) {
		while (file[line][pos]) {
			if (file[line][pos] == '}') {
				end = line;
				break ;
			}
			pos++;
		}
		pos = 0;
		line++;
	}
	while (*start < end) {
		for (size_t i = 0; i < locations.locParams.size(); i++) {
			if (!easyFind(locations.locParams[i], file[*start])) {
				fillLocation(locations.locParams[i], file[*start], _servers[index].getLocations()[indexLocation], i);
			}
		}
		(*start)++;
	}
}

void ft::Parser::serversInfo(ssize_t index, std::vector<std::string> file, size_t start, size_t end) {
	ft::ValidConfigKeys rootParams;
	std::string param;

	while (start < end) {
		for (size_t i = 0; i < rootParams.servParams.size(); i++) {
			if (!easyFind(rootParams.servParams[i], file[start]))
				fillConfig(rootParams.servParams[i], file[start], index, i);
			else if (!easyFind("location", file[start])) {
				locationsInfo(file, index, &start, end);
			}
		}
		start++;
	}
}

int ft::Parser::isEmptyLine(std::string str) {
	size_t i = 0;

	while (str[i]) {
		if (str[i] != ' ' and str[i] != '\t' and str[i] != '\n')
			return -1;
		i++;
	}
	return 0;
}

void ft::Parser::cleanSpaces(std::string *string) {
	size_t i = 0;

	if ((*string)[i] == ' ' or (*string)[i] == '\t') {
		while ((*string)[i] == ' ' or (*string)[i] == '\t')
			i++;
	}
	*string = (*string).substr(i, (*string).size());
	i = (*string).size();
	if ((*string)[i - 1] == ' ' or (*string)[i - 1] == '\t' or (*string)[i - 1] == ';') {
		while ((*string)[i - 1] == ' ' or (*string)[i - 1] == '\t' or (*string)[i - 1] == ';')
			i--;
	}
	if (i < (*string).size())
		*string = (*string).substr(0, i);
}

std::vector<std::string>  ft::Parser::lineBreaks(std::string string) {
	std::vector<std::string> vec;
	std::string newString;
	std::string tmp;
	size_t i = 0;

	while (string[i]) {
		if (string[i] == '}' or string[i] == '{') {
			tmp = string.substr(i, string.size());
			newString = string.substr(0, i);
			cleanSpaces(&newString);
			if (!newString.empty())
				vec.push_back(newString);
			newString = tmp.substr(0, 1);
			vec.push_back(newString);
			newString = tmp.substr(1, tmp.size());
			cleanSpaces(&newString);
			if (!newString.empty())
				vec.push_back(newString);
			tmp.clear();
			newString.clear();
		}
		i++;
	}
	return vec;
}

std::vector<std::string> ft::Parser::checkAndClean(std::string argv) {
	std::ifstream file(argv);
	std::string buffer;
	std::vector<std::string> tmp;
	std::vector<std::string> config;

	if (argv.substr((argv.size() - strlen(".conf")), strlen(".conf")) != ".conf" or (!(file.is_open())))
		throw std::invalid_argument("Parser error: wrong file type");
	else {
		while (!file.eof()) {
			getline(file, buffer);
			if (buffer[0] == '#' or buffer.empty()) {
				continue;
			}
			else if (buffer.find('#')) {
				buffer = buffer.substr(0, buffer.find('#'));
				if (!isEmptyLine(buffer)) {
					continue ;
				}
			}
			tmp = lineBreaks(buffer);
			if (!tmp.empty()) {
				for (size_t i = 0; i < tmp.size(); i++) {
					buffer = tmp[i];
					tmp[i].clear();
					config.push_back(buffer);
				}
			}
			else {
				cleanSpaces(&buffer);
				config.push_back(buffer);
			}
			buffer.clear();
		}
	}
	config.push_back("");
	file.close();
	return config;
}

int ft::Parser::validKeys(std::string config) {
	if (_validConfigParams.servKeys.find(config)->first == config)
		return 0;
	return -1;
}

void ft::Parser::checkAllKeys(std::vector<std::string> config) {
	std::vector<std::string>::iterator it;
	std::string newStr;
	size_t i = 0;

	it = config.begin();
	while (it < config.end()) {
		i = 0;
		while ((*it)[i] != ' ' and (*it)[i] != '\t' and (*it)[i])
			i++;
		newStr = (*it).substr(0, i);
		if (validKeys(newStr))
			throw std::invalid_argument("Parser error: incorrect key in the config");
		it++;
	}
}
