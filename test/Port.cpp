#include "Port.hpp"

ft::Port::Port() : port_(0), host_(htonl(INADDR_ANY)){

}

ft::Port::~Port() {

}

void ft::Port::setPort_(const u_short &port) {
    port_ = port;
}

const u_short &ft::Port::getPort() const {
    return port_;
}

void ft::Port::setHost_(const in_addr_t &host) {
    host_ = host;
}

const in_addr_t &ft::Port::getHost() const {
    return host_;
}

void ft::Port::init(const u_short &port, const in_addr_t &host) {
    setPort_(port);
    setHost_(host);
}
