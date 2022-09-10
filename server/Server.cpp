#include "Server.hpp"

ft::Server::Server() {
    serverName_ = "";
    host_ = htonl(INADDR_ANY);
    port_ = 0;
    maxBodySize_ = 0;
}

ft::Server &ft::Server::operator=(const Server &other) {
    ALocation::operator=(other);
    serverName_ = other.serverName_;
    host_ = other.host_;
    port_ = other.port_;
    maxBodySize_ = other.maxBodySize_;
    locations_.insert(locations_.end(), other.locations_.begin(), other.locations_.end());
    return *this;
}

ft::Server::~Server() {

}

void ft::Server::setPort(const int &port) {
    port_ = htons(port);
}

const u_short &ft::Server::getPort() const {
    return port_;
}

void ft::Server::setHost(const std::string &host) {
    host_ = inet_addr(host.c_str());
}

const in_addr_t &ft::Server::getHost() const {
    return host_;
}

void ft::Server::setServerName(const std::string &serverName) {
    serverName_ = serverName;
}

const std::string &ft::Server::getServerName() const {
    return serverName_;
}

void ft::Server::setMaxBodySize(const int &maxBodySize) {
    maxBodySize_ = maxBodySize;
}

const int &ft::Server::getMaxBodySize() const {
    return maxBodySize_;
}

std::vector<ft::Location> &ft::Server::getLocations() {
    return locations_;
}
