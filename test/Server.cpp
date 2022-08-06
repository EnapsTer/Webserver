//
// Created by Janelle Kassandra on 3/7/22.
//

#include "Server.hpp"

ft::Server::Server() {
    _port = 0;
    _host = htonl(INADDR_ANY);
    _serverName = "";
    _maxBodySize = 0;
//    _autoIndex = false;
}

ft::Server &ft::Server::operator=(const Server &other) {
    ALocation::operator=(other);
    _port = other._port;
    _host = other._host;
    _serverName = other._serverName;
    _maxBodySize = other._maxBodySize;
//    _autoIndex = other._autoIndex;
    _locations.insert(_locations.end(), other._locations.begin(), other._locations.end());
    return *this;
}

ft::Server::~Server() {

}

void ft::Server::setPort(const int &port) {
    _port = htons(port);
}

const u_short &ft::Server::getPort() const {
    return _port;
}

void ft::Server::setHost(const std::string &host) {
    _host = inet_addr(host.c_str());
}

const in_addr_t &ft::Server::getHost() const {
    return _host;
}

void ft::Server::setServerName(const std::string &serverName) {
    _serverName = serverName;
}

const std::string &ft::Server::getServerName() const {
    return _serverName;
}

void ft::Server::setMaxBodySize(const int &maxBodySize) {
    _maxBodySize = maxBodySize;
}

const int &ft::Server::getMaxBodySize() const {
    return _maxBodySize;
}

//void ft::Server::setAutoIndex(const bool &autoIndex) {
//    _autoIndex = autoIndex;
//}
//
//const bool &ft::Server::getAutoIndex() const {
//    return _autoIndex;
//}

std::vector<ft::Location> &ft::Server::getLocations() {
    return _locations;
}
