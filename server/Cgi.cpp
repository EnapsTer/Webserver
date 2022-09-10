#include "Responder.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/param.h>
#include <ctime>
#include <signal.h>

void        ft::Responder::cgiHandler(int fd) {
    fd_data &fd_dat = fdHostMap_[fd];

    if (not fd_dat._wait_from_cgi) {

        char c;
        std::string rootPath = "www/" + fd_dat.server_->getRoot();
        std::stringstream ss(fd_dat.url_);

        std::string pathScript = rootPath;
        while (ss.peek() != '.' and ss >> c) {
            pathScript += c;
        }

        if (ss.eof()) {
            fd_dat.status_ = Send;
            return;
        }

        std::string extension = "";
        while (ss.peek() != '?' and ss.peek() != '/' and ss >> c) {
            extension += c;
        }
        if (extension != ".py" and extension != ".sh") {
            fd_dat.status_ = Send;
            return;
        }
        pathScript += extension;

        std::string pathInfo = "";
        while (ss.peek() != '?' and ss >> c) {
            pathInfo += c;
        }

        char dir[MAXPATHLEN];
        getwd(dir);
        std::string pathTranslated = dir;
        pathTranslated += "/" + rootPath + pathInfo;

        if (ss.peek() == '?') {
            ss >> c;
        }

        std::string queryString = "";
        while (ss >> c) {
            queryString += c;
        }

        char *cmd[3];
        for (int i = 0; i < 3; ++i) {
            cmd[i] = NULL;
        }
        if (extension == ".py") {
            cmd[0] = strdup(fd_dat.location_->getPathToBinPy().c_str());
        } else {
            cmd[0] = strdup(fd_dat.location_->getPathToBinSh().c_str());
        }
        cmd[1] = strdup(pathScript.c_str());

        if (cmd[0] == NULL or cmd[1] == NULL) {
            free(cmd[0]);
            free(cmd[1]);
            fd_dat.responseCode_ = 500;
            sendResponse(fd);
            return;
        }

        size_t envLen = fd_dat.requestHeadMap_.size();
        char *env[envLen + 4];
        for (size_t i = 0; i < envLen + 4; ++i) {
            env[i] = NULL;
        }
        std::string key;
        int j = 0;
        for (std::map<std::string, std::string>::iterator it = fd_dat.requestHeadMap_.begin();
             it != fd_dat.requestHeadMap_.end(); ++it) {
            key = it->first;
            for (size_t i = 0; i < key.size(); ++i) {
                if (key[i] == '-') {
                    key[i] = '_';
                } else if (key[i] == ':') {
                    key[i] = '=';
                } else {
                    key[i] = std::toupper(key[i]);
                }
            }
            key += it->second;
            env[j++] = strdup(key.c_str());
        }
        key = "PATH_INFO=" + pathInfo;
        env[j++] = strdup(key.c_str());
        key = "PATH_TRANSLATED=" + pathTranslated;
        env[j++] = strdup(key.c_str());
        key = "QUERY_STRING=" + queryString;
        env[j++] = strdup(key.c_str());

        for (int i = 0; i < j; ++i) {
            if (env[i] == NULL) {
                free(cmd[0]);
                free(cmd[1]);
                for (int k = 0; k < i; ++k) {
                    free(env[k]);
                }
                fd_dat.responseCode_ = 500;
                sendResponse(fd);
                return;
            }
        }

        fd_dat._outName = rootPath + "/outCgi" + intToString(fd_dat.fd);

        if (fd_dat.requestType_ == "POST") {
            fd_dat._inName = rootPath + "/inCgi" + intToString(fd_dat.fd);
        }

        fd_dat._pid = fork();

        if (fd_dat._pid < 0) {
            perror("fork");
            free(cmd[0]);
            free(cmd[1]);
            for (int i = 0; i < j; ++i) {
                free(env[i]);
            }
            fd_dat.responseCode_ = 500;
            sendResponse(fd);
            return;
        }

        if (fd_dat._pid == 0) {
            int outCgiFd = open(fd_dat._outName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
            if (outCgiFd < 0) {
                perror("open OUT");
                exit(EXIT_FAILURE);
            }
            if (dup2(outCgiFd, STDOUT_FILENO) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }
             if (fd_dat.requestType_ == "POST") {
                 int inCgiFd = open(fd_dat._inName.c_str(), O_RDONLY, 0);
                 if (inCgiFd < 0) {
                     perror("open IN");
                     exit(EXIT_FAILURE);
                 }
                 if (dup2(inCgiFd, STDIN_FILENO) == -1) {
                     perror("dup2");
                     exit(EXIT_FAILURE);
                 }
             }
            close(outCgiFd);
            if (execve(cmd[0], cmd, env) == -1) {
                perror("execve");
                exit(EXIT_FAILURE);
            }
        }

        free(cmd[0]);
        free(cmd[1]);
        for (int i = 0; i < j; ++i) {
            free(env[i]);
        }
        std::time(&fd_dat._time);
    }

    int status;
    if (waitpid(fd_dat._pid, &status, WNOHANG) != fd_dat._pid) {
        fd_dat._wait_from_cgi = true;
        time_t cur_time;
        std::time(&cur_time);
        if (std::difftime(cur_time, fd_dat._time) > 30) {
            kill(fd_dat._pid, SIGKILL);
            fd_dat._wait_from_cgi = false;
            fd_dat.responseCode_ = 500;
            unlink(fd_dat._outName.c_str());
            if (fd_dat.requestType_ == "POST") {
                unlink(fd_dat._inName.c_str());
            }
            sendResponse(fd);
            return;
        }
        return;
    }

    if (fd_dat.requestType_ == "POST") {
        unlink(fd_dat._inName.c_str());
    }
    fd_dat._wait_from_cgi = false;

    if (not WIFEXITED(status) or WEXITSTATUS(status) != EXIT_SUCCESS) {
        std::cout << "Exit status error" << std::endl;
        fd_dat.responseCode_ = 500;
        unlink(fd_dat._outName.c_str());
        sendResponse(fd);
        return;
    }

    std::ifstream output(fd_dat._outName);
    if (not output.good()) {
        fd_dat.responseCode_ = 500;
        unlink(fd_dat._outName.c_str());
        sendResponse(fd);
        return;
    }
    output.seekg(0, output.end);
    int len = output.tellg();
    output.seekg(0, output.beg);
    char buff[len + 1];
    bzero(buff, len + 1);
    output.read(buff, len);
    output.close();
    unlink(fd_dat._outName.c_str());

    fd_dat.response_ += "200 OK\n";
    std::string tmp(buff);
    std::size_t body_size = tmp.find("\n\n");

    if (body_size == std::string::npos) {
        body_size = 0;
    } else {
        body_size = tmp.size() - body_size - 2;
    }

    if (body_size != 0) {
        fd_dat.response_ += "Content-Length: ";
        fd_dat.response_ += intToString(body_size);
        fd_dat.response_ += "\n";
    }

    fd_dat.response_ += tmp;
    if (send(fd, fd_dat.response_.c_str(), fd_dat.response_.size(), 0) < 0) {
        FD_CLR(fd, &writeMaster_);
        FD_CLR(fd, &master_);
        close(fd);
        fd_dat.status_ = ClosedFd;
        return;
    }
    FD_CLR(fd, &writeMaster_);
    fd_dat.bodyLength_ = 0;

    if (fd_dat.requestHeadMap_["Connection:"] == "close") {
        FD_CLR(fd, &master_);
        close(fd);
        fd_dat.status_ = ClosedFd;
        return;
    }
    fd_dat.status_ = WithoutSession;
}
