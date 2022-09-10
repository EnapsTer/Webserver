#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <vector>
#include <list>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "Parser.hpp"
#include "Responder.hpp"

namespace ft {

    struct Listener {
        int socket;
        u_short port;
        in_addr_t host;
    };

    class WebServ : public Parser {

    private:

        std::vector<struct Listener> listenSockets_;
        std::list<int> clientSockets_;
        int socketNumber_;
        Responder responder_;

        WebServ();

        void initializeListenSocket_(const int &i);
        void createClientSocket_(const Listener &socket);
        void createListenSockets_();

    public:
        void run();

        WebServ(const char *config);
        virtual ~WebServ();
    };

}

#endif
