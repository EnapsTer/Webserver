
//
// Created by Janelle Kassandra on 3/7/22.
//

#ifndef VOVA_WEBSERV_HPP
#define VOVA_WEBSERV_HPP

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

        std::vector<struct Listener> _listenSockets;
        std::list<int> _clientSockets;
        int _num;
        Responder _responder;

        WebServ();

        void _initListenSocket(const int &i);

        void _createListenSockets();

        void _createClientSocket(const Listener &fd);

    public:

        WebServ(const char *config);

        virtual ~WebServ();


        void run();
    };

}

#endif //VOVA_WEBSERV_HPP
