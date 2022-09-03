#ifndef PORT_HPP
#define PORT_HPP

#include <iostream>
#include <arpa/inet.h>

namespace ft {
    class Port {

    private:

        u_short port_;
        in_addr_t host_;

        void setPort_(const u_short &port);
        void setHost_(const in_addr_t &host);

    public:

        Port();

        virtual ~Port();

        const u_short &getPort() const;
        const in_addr_t &getHost() const;

        void init(const u_short &port, const in_addr_t &host);
    };

}

#endif
