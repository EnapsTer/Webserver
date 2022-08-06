//
// Created by Janelle Kassandra on 3/7/22.
//

#include "WebServ.hpp"

ft::WebServ::WebServ(const char* config) : Parser(config), _responder(_servers) {
    _num = 0;
}

ft::WebServ::~WebServ() {

}

void ft::WebServ::_initListenSocket(const int& i) {
    struct sockaddr address;
    struct sockaddr_in& addressIn = reinterpret_cast<struct sockaddr_in&>(address);

    memset(reinterpret_cast<char *>(&address), 0, sizeof(struct sockaddr));

    addressIn.sin_family = AF_INET;
    addressIn.sin_port = _ports[i].getPort();
    addressIn.sin_addr.s_addr = _ports[i].getHost();


    _listenSockets[i].port = addressIn.sin_port;
    _listenSockets[i].host = addressIn.sin_addr.s_addr;
    _listenSockets[i].socket = socket(AF_INET, SOCK_STREAM, 0);

std::cout << "listen socket " << _listenSockets[i].socket << " addr " << addressIn.sin_port << " " << addressIn.sin_addr.s_addr <<  std::endl;


    int opt = 1;
    setsockopt(_listenSockets[i].socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (::bind(_listenSockets[i].socket, &address, sizeof(address))) {
        perror("bind");
        exit(1);
    }

    if (listen(_listenSockets[i].socket, 32)) {
        perror("listen");
        exit(1);
    }

    FD_SET(_listenSockets[i].socket, &_responder.getMaster());

    if (_listenSockets[i].socket > _num) {
        _num = _listenSockets[i].socket;
    }
}

void ft::WebServ::_createListenSockets() {
    int size = _ports.size();
    _listenSockets.resize(size);
    for (int i = 0; i < size; ++i) {
        _initListenSocket(i);
    }
}

void ft::WebServ::_createClientSocket(const Listener& socket) {

    struct sockaddr address;
    struct sockaddr_in& addressIn = reinterpret_cast<struct sockaddr_in&>(address);
    socklen_t addressLen;

    addressIn.sin_port = socket.port;
    addressIn.sin_addr.s_addr = socket.host;
    u_short port = addressIn.sin_port;
    in_addr_t host = addressIn.sin_addr.s_addr;
    
    int fd = accept(socket.socket, &address, &addressLen);

    if (fd < 0) {
        return ;
    }

    fcntl(fd, F_SETFL, O_NONBLOCK);
    FD_SET(fd, &_responder.getMaster());
    _responder.add_to_map(fd, port, host);
//    std::cout << "port WBS: " << addressIn.sin_port << "     ip " << addressIn.sin_addr.s_addr << std::endl;

    if (fd > _num) {
        _num = fd;
    }
    _clientSockets.push_back(fd);
}

void ft::WebServ::run() {
    _createListenSockets();
    fd_set readFd;
    fd_set writeFd;

    while (_num) {
        readFd = _responder.getMaster();
        writeFd = _responder.getWriteMaster();

        int res = select(_num + 1, &readFd, &writeFd, 0, 0);
        if (res <= 0) {
            continue ;
        }

        int listenSize = _listenSockets.size();
        for (int i = 0; i < listenSize; ++i) {
           if (FD_ISSET(_listenSockets[i].socket, &readFd)) {
               _createClientSocket(_listenSockets[i]);
           }
        }

        for (std::list<int>::iterator it = _clientSockets.begin(); it != _clientSockets.end(); ++it) {
            if (FD_ISSET(*it, &readFd) and not _responder.is_ready_to_send(*it)) {
               _responder.action(*it);
                if (_responder.is_to_del(*it)) {
//                    std::cout << "DELETED" << std::endl;
                    _responder.del_from_map(*it);
                    _clientSockets.erase(it);
                }
                continue ;
           }
            if (FD_ISSET(*it, &writeFd) and _responder.is_ready_to_send(*it)) {
                _responder.action(*it);
                if (_responder.is_to_del(*it)) {
//                    std::cout << "DELETED" << std::endl;
                    _responder.del_from_map(*it);
                    _clientSockets.erase(it);
                }
            }
        }
    }
}

