#ifndef SERVERPARAMETERS_HPP
#define SERVERPARAMETERS_HPP

#include <algorithm>
#include <iostream>
#include <vector>
#include <map>

namespace ft {
	struct ValidConfigKeys {
		std::map<std::string, int>			serverKeys;
		std::vector<std::string>			serverParameters;
		std::vector<std::string>			localParameters;
		std::vector<std::string>			errorPage;
		std::map<std::string, std::string>	errorsMap;

		ValidConfigKeys(int responder);
		ValidConfigKeys();
		~ValidConfigKeys();
	};
}

#endif
