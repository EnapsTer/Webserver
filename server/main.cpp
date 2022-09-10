#include <iostream>
#include "WebServ.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Error: need one argument with path to configuration file." << std::endl;
        return 0;
    }

    ft::WebServ webServ(argv[1]);

    try {
        webServ.parse();
        webServ.run();
    }
	catch (std::invalid_argument& e) {
		std::cout << e.what() << std::endl;
	}
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
