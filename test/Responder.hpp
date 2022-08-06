#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <list>
#include <sys/select.h>
#include <fcntl.h>
#include "./Server.hpp"
#include <unistd.h>
#include <cstdio>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "ServerParameters.hpp"
#define BUFFER 2048

namespace ft
{

struct fd_data
{
    in_addr_t           _ip;
    u_short               _port;
    int                 _status;
    Server*             _server;
    Location            *_location;
    std::fstream        _iff;
    int                 _len_body;
	std::string              _filename;
	std::string              _resp;
	std::string              _url;
	std::string              _request_type;
	std::string              _http11;
	std::map<std::string, std::string> _request_head_map;
    int                 _code_resp;
    int                 fd;
	std::ofstream            _outdata;
	std::string              _error_page;
	std::string				_autoIndex;
    int                 _wasreaded;
    bool                _is_chunked;
    int                 _chunk_ostatok;
	std::string              _hex;

    bool                _wait_from_cgi;
    std::string         _outName;
    std::string         _inName;
    pid_t               _pid;
    time_t              _time;

};

enum SessionStatus
{ Nosession, Readbody, Send, Sendbody, Cgi, Closefd, AutoIndex};

class Responder
{
    public:
         Responder(std::vector<Server> & vec);

		void        action(int fd);
        void        make_session(int fd);
        void        close_session(int fd);
        void        read_post_body(int fd);
        void        send_resp(int fd);
        void        send_resp_body(int fd);
        fd_set&     getMaster();
        fd_set&     getWriteMaster();
        bool        is_ready_to_send(int fd);
        bool        is_ready_to_read_body(int fd);
        void        add_to_map(const int& fd, const u_short& port, const in_addr_t& host);
        bool        is_to_del(int fd);
        void        del_from_map(int fd);
        void        parse_request(fd_data &fd_dat);
        void        find_server(fd_data &fd_dat);
        void        cgi_handler(int fd);
		bool		s_dir(const char *path);

		std::string errorInsertion (std::string key, std::string value, std::string str);
		std::string makeErrorPage (int errorCode);
		void        make_Autoindex(fd_data &fd_dat);

    private:
	std::map<int, fd_data>                       _fd_host_map;
        char                                    _buff[BUFFER];
	std::vector<Server>&                         _servers;
        fd_set                                  _master;
        fd_set                                 writeMaster;
        ft::ValidConfigKeys                   _valid_conf;
 };
}

std::string int_to_string(int a);
int	hexToInt(std::string str);
