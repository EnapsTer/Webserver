//
// Created by Janelle Kassandra on 3/7/22.
//

#ifndef VOVA_SERVER_HPP
#define VOVA_SERVER_HPP

#include <iostream>
#include <vector>
#include <arpa/inet.h>
#include "Location.hpp"
#include "ALocation.hpp"

namespace ft {

class Server : public ft::ALocation {

    private:

        u_short _port;
        in_addr_t _host;
        std::string _serverName;
        int _maxBodySize;
//        bool _autoIndex;
        std::vector<Location> _locations;

    public:

        Server();
        Server &operator=(const Server &other);
        virtual ~Server();

        void setPort(const int &port);
        const u_short &getPort() const;

        void setHost(const std::string &host);
        const in_addr_t &getHost() const;

        void setServerName(const std::string &serverName);
        const std::string &getServerName() const;

        void setMaxBodySize(const int &maxBodySize);
        const int &getMaxBodySize() const;

//        void setAutoIndex(const bool &autoIndex);
//        const bool &getAutoIndex() const;

        std::vector<Location> &getLocations();

    };

}

#endif //VOVA_SERVER_HPP
