#include "WebServ.hpp"

ft::WebServ::WebServ(const char* config) : Parser(config), responder_(servers_) {
    socketNumber_ = 0;
}

ft::WebServ::~WebServ() {

}

void ft::WebServ::initializeListenSocket_(const int& i) {
    struct sockaddr address;
    struct sockaddr_in& addressIn = reinterpret_cast<struct sockaddr_in&>(address);

    memset(reinterpret_cast<char *>(&address), 0, sizeof(struct sockaddr));

    addressIn.sin_addr.s_addr = ports_[i].getHost();
    addressIn.sin_port = ports_[i].getPort();
    addressIn.sin_family = AF_INET;


    listenSockets_[i].socket = socket(AF_INET, SOCK_STREAM, 0);
    listenSockets_[i].host = addressIn.sin_addr.s_addr;
    listenSockets_[i].port = addressIn.sin_port;

    std::cout << "Listening socket " << listenSockets_[i].socket << " on address " << addressIn.sin_port << " " << addressIn.sin_addr.s_addr << std::endl;


    int opt = 1;
    setsockopt(listenSockets_[i].socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (::bind(listenSockets_[i].socket, &address, sizeof(address))) {
        perror("bind");
        exit(1);
    }

    if (listen(listenSockets_[i].socket, 32)) {
        perror("listen");
        exit(1);
    }

    FD_SET(listenSockets_[i].socket, &responder_.getMaster());

    if (listenSockets_[i].socket > socketNumber_) {
        socketNumber_ = listenSockets_[i].socket;
    }
}

void ft::WebServ::createListenSockets_() {
    int size = ports_.size();

    listenSockets_.resize(size);
    for (int socket = 0; socket < size; socket++)
        initializeListenSocket_(socket);
}

void ft::WebServ::createClientSocket_(const Listener& socket) {

    struct sockaddr address;
    struct sockaddr_in& addressIn = reinterpret_cast<struct sockaddr_in&>(address);
    socklen_t addressLength;

    addressIn.sin_addr.s_addr = socket.host;
    addressIn.sin_port = socket.port;
    in_addr_t host = addressIn.sin_addr.s_addr;
    u_short port = addressIn.sin_port;

    int fd = accept(socket.socket, &address, &addressLength);

    if (fd < 0)
        return ;

    fcntl(fd, F_SETFL, O_NONBLOCK);
    FD_SET(fd, &responder_.getMaster());
    responder_.addToMap(fd, port, host);

    if (fd > socketNumber_)
        socketNumber_ = fd;
    clientSockets_.push_back(fd);
}

void ft::WebServ::run() {
    createListenSockets_();
    fd_set readFd;
    fd_set writeFd;

    while (socketNumber_) {
        readFd = responder_.getMaster();
        writeFd = responder_.getWriteMaster();

        int res = select(socketNumber_ + 1, &readFd, &writeFd, 0, 0);
        if (res <= 0) {
            continue ;
        }

        int listenSize = listenSockets_.size();
        for (int i = 0; i < listenSize; ++i) {
           if (FD_ISSET(listenSockets_[i].socket, &readFd)) {
               createClientSocket_(listenSockets_[i]);
           }
        }

        for (std::list<int>::iterator it = clientSockets_.begin(); it != clientSockets_.end(); it++) {
            if (FD_ISSET(*it, &readFd) && !responder_.isReadyToSend(*it)) {
               responder_.action(*it);
                if (responder_.isToDelete(*it)) {
                    responder_.deleteFromMap(*it);
                    clientSockets_.erase(it);
                }
                continue ;
           }
            if (FD_ISSET(*it, &writeFd) && responder_.isReadyToSend(*it)) {
                responder_.action(*it);
                if (responder_.isToDelete(*it)) {
                    responder_.deleteFromMap(*it);
                    clientSockets_.erase(it);
                }
            }
        }
    }
}

