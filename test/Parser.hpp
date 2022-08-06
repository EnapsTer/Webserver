#ifndef PARSER_HPP
#define PARSER_HPP

#include "Server.hpp"
#include "Port.hpp"
#include <vector>
#include "ServerParameters.hpp"

enum ValidRootKeys {
	Host_port,
	Server_name,
	Autoindex,
	Root,
	Index_page,
	Methods,
	Client_max_body_size,
	Error_page,
	UploadPath,
	Redirection
};

enum ValidLocKeys {
	Location_name,
	Location_methods,
	Location_root,
	Location_redirection,
	Location_error_page,
	Bin_path_py,
	Bin_path_sh,
	Path_cgi,
	Location_index,
	LocationUploadPath,
	LocationAutoindex
};

namespace ft {
	class Parser {
	private:

		Parser();

		ft::ValidConfigKeys _validConfigParams;

		int easyFind(std::string word, std::string line);
		std::vector<std::string> splitString(std::string key, std::string line);
		std::pair<int,std::string> fillErrorPage(std::vector<std::string> value);
		void cleanSpaces(std::string *string);
		int checkBrackets(std::vector<std::string> file, size_t *line);
		int isEmptyLine(std::string str);
		std::vector<std::string> lineBreaks(std::string string);
		std::vector<std::string> checkAndClean(std::string argv);

		int validKeys(std::string config);
		std::vector<std::string> checkHost(std::string host);
		void checkAllKeys(std::vector<std::string> config);

		std::string errorInsertion (std::string key, std::string value, std::string str);
		int whatIsIt(std::string value);

		void serversInfo(ssize_t i, std::vector<std::string> file, size_t start, size_t end);
		void fillConfig(std::string key, std::string line, ssize_t index, size_t i);
		void fillHostPort(std::string key, std::string line, ssize_t index);
		void fillServerName(std::string key, std::string line, ssize_t index);
		void fillAutoindex(std::string key, std::string line, ssize_t index);
		void fillServerRoot(std::string key, std::string line, ssize_t index);
		void fillIndex(std::string key, std::string line, ssize_t index);
		void fillRootMethods(std::string key, std::string line, ssize_t index);
		void fillRootMaxBodySize(std::string key, std::string line, ssize_t index);
		void fillRootErrorPages(std::string key, std::string line, ssize_t index);
		void fillRootRedirection(std::string key, std::string line, ssize_t index);
		void fillUploadPath(std::string key, std::string line, ssize_t index);

		void locationsInfo(std::vector<std::string> file, ssize_t index, size_t *start, size_t end);
		void fillLocation(std::string key, std::string line, ft::Location& location, size_t i);
		void fillLocationName(std::string key, std::string line, ft::Location& location);
		void fillLocationMethods(std::string key, std::string line, ft::Location& location);
		void fillLocationRoot(std::string key, std::string line, ft::Location& location);
		void fillLocationRedirection(std::string key, std::string line, ft::Location& location);
		void fillLocationErrorsPages(std::string key, std::string line, ft::Location& location);
		void fillLocationBinPathPy(std::string key, std::string line, ft::Location& location);
		void fillLocationBinPathSh(std::string key, std::string line, ft::Location& location);
		void fillLocationPathCgi(std::string key, std::string line, ft::Location& location);
		void fillLocationIndex(std::string key, std::string line, ft::Location& location);
		void fillLocationUploadPath(std::string key, std::string line, ft::Location& location);
		void fillLocationAutoindex(std::string key, std::string line, ft::Location& location);

	protected:

		const char* _config;
		std::vector<Port> _ports;
		std::vector<Server> _servers;

	public:

		Parser(const char* config);
		virtual ~Parser();

		void parse();
		const std::vector<Server>& getServers() const;
	};
}

#endif
