#ifndef SERVER_HPP
#define SERVER_HPP

#include "Location.hpp"
#include "ALocation.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <vector>

namespace ft {

class Server : public ft::ALocation {

    private:

        std::string serverName_;
        in_addr_t host_;
        u_short port_;
        int maxBodySize_;
        std::vector<Location> locations_;

    public:

        Server();
        Server &operator=(const Server &other);

        virtual ~Server();

        const std::string &getServerName() const;
        void setServerName(const std::string &serverName);

        const in_addr_t &getHost() const;
        void setHost(const std::string &host);

        const u_short &getPort() const;
        void setPort(const int &port);

        const int &getMaxBodySize() const;
        void setMaxBodySize(const int &maxBodySize);

        std::vector<Location> &getLocations();

    };

}

#endif
