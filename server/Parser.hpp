#ifndef PARSER_HPP
#define PARSER_HPP

#include "Server.hpp"
#include "Port.hpp"
#include <vector>
#include "ServerParameters.hpp"

enum ValidRootKeys {
	Host_port,
	Server_name,
	AutoIndex,
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
    LocationAutoIndex
};

namespace ft {
	class Parser {
	private:

		Parser();

		ft::ValidConfigKeys _validConfigParams;

		std::vector<std::string> splitByWhitespaces(std::string key, std::string line);
		int isWord(std::string word, std::string line);
		void cleanWhitespaces(std::string *line);
		int isValidBrackets(std::vector<std::string> file, size_t *line);
		int isEmptyLine(std::string str);
		std::vector<std::string> lineBreaks(std::string line);
		std::vector<std::string> checkAndClean(std::string file_name);
		std::pair<int,std::string> fillErrorPage(std::vector<std::string> value);
		std::vector<std::string> checkHost(std::string host);
		void checkKeys(std::vector<std::string> config);
		int isValidKeys(std::string config);
		int findDot(std::string value);

		void serversInformation(ssize_t index, std::vector<std::string> file, size_t start, size_t end);
		void fillConfig(std::string key, std::string line, ssize_t index, size_t i);
		void fillHostAndPort(std::string key, std::string line, ssize_t index);
		void fillAutoIndex(std::string key, std::string line, ssize_t index);
		void fillServerName(std::string key, std::string line, ssize_t index);
		void fillServerRoot(std::string key, std::string line, ssize_t index);
		void fillRootMaxBodySize(std::string key, std::string line, ssize_t index);
		void fillRootMethods(std::string key, std::string line, ssize_t index);
		void fillIndex(std::string key, std::string line, ssize_t index);
		void fillRootErrorPages(std::string key, std::string line, ssize_t index);
		void fillUploadPath(std::string key, std::string line, ssize_t index);

		void locationsInformation(std::vector<std::string> file, ssize_t index, size_t *start, size_t end);
		void fillMethods(std::string key, std::string line, ft::Location& location);
		void fillLocationRedirect(std::string key, std::string line, ft::Location& location);
		void fillLocationRoot(std::string key, std::string line, ft::Location& location);
		void fillLocation(std::string key, std::string line, ft::Location& location, size_t i);
		void fillLocationBinPathPy(std::string key, std::string line, ft::Location& location);
		void fillLocationErrorsPages(std::string key, std::string line, ft::Location& location);
		void fillLocationName(std::string key, std::string line, ft::Location& location);
		void fillLocationAutoIndex(std::string key, std::string line, ft::Location& location);
		void fillLocationUploadPath(std::string key, std::string line, ft::Location& location);
		void fillLocationIndex(std::string key, std::string line, ft::Location& location);
		void fillLocationBinPathSh(std::string key, std::string line, ft::Location& location);

	protected:

		const char* config_;
		std::vector<Port> ports_;
		std::vector<Server> servers_;

	public:

		Parser(const char* config);
		virtual ~Parser();

		void parse();
	};
}

#endif
