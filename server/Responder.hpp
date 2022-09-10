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
    in_addr_t           ip_;
    u_short               port_;
    int                 status_;
    Server*             server_;
    Location            *location_;
    std::fstream        iff_;
    int                 bodyLength_;
	std::string              filename_;
	std::string              response_;
	std::string              url_;
	std::string              requestType_;
	std::string              http11_;
	std::map<std::string, std::string> requestHeadMap_;
    int                 responseCode_;
    int                 fd;
	std::ofstream            _outdata;
	std::string              _error_page;
	std::string				autoIndex_;
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

enum SessionStatus {
    WithoutSession,
    ReadBody,
    Send,
    SendBody,
    Cgi,
    ClosedFd,
    AutoIndex
};

class Responder
{
    public:
         Responder(std::vector<Server> & vec);

        void closeSession(int fd);
        void createSession(int fd);
		void action(int fd);
        void sendResponseBody(int fd);
        fd_set& getWriteMaster();
        void readPostBody(int fd);
        fd_set& getMaster();
        void addToMap(const int& fd, const u_short& port, const in_addr_t& host);
        void sendResponse(int fd);
        bool isReadyToSend(int fd);
        void cgiHandler(int fd);
        void parseRequest(fd_data &fd_dat);
        bool isToDelete(int fd);
		static bool sDir(const char *path);
        void findServer(fd_data &fd_dat);
        void deleteFromMap(int fd);
		void createAutoIndex(fd_data &fd_dat);

		std::string makeErrorPage (int errorCode);
		std::string errorInsertion (std::string key, std::string value, std::string str);

    private:
        std::map<int, fd_data> fdHostMap_;
        std::vector<Server>& servers_;
        ft::ValidConfigKeys validConfig_;
        fd_set master_;
        fd_set writeMaster_;
        char buff_[BUFFER];
 };
}

std::string intToString(int a);
int	hexToInt(std::string str);
