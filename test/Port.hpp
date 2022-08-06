//
// Created by Janelle Kassandra on 3/8/22.
//

#ifndef VOVA_PORT_HPP
#define VOVA_PORT_HPP

#include <iostream>
#include <arpa/inet.h>
namespace ft {

    class Port {

    private:

        u_short _port;
        in_addr_t _host;

        void _setPort(const u_short &port);
        void _setHost(const in_addr_t &host);

    public:

        Port();

        virtual ~Port();

        const u_short &getPort() const;
        const in_addr_t &getHost() const;

        void init(const u_short &port, const in_addr_t &host);
    };

}


#endif //VOVA_PORT_HPP
