//
// Created by Janelle Kassandra on 3/8/22.
//

#include "Port.hpp"

ft::Port::Port() : _port(0), _host(htonl(INADDR_ANY)){

}

ft::Port::~Port() {

}

void ft::Port::_setPort(const u_short &port) {
    _port = port;
}

const u_short &ft::Port::getPort() const {
    return _port;
}

void ft::Port::_setHost(const in_addr_t &host) {
    _host = host;
}

const in_addr_t &ft::Port::getHost() const {
    return _host;
}

void ft::Port::init(const u_short &port, const in_addr_t &host) {
    _setPort(port);
    _setHost(host);
}
