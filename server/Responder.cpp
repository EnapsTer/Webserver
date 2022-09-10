#include "Responder.hpp"

ft::Responder::Responder(std::vector<Server> & vec): servers_(vec), validConfig_(1)
{
        FD_ZERO(&master_);
        FD_ZERO(&writeMaster_);
}

void ft::Responder::parseRequest(fd_data &fd_dat)
{
    std::string s(buff_);
    std::istringstream in(s);
    in >> fd_dat.requestType_;
    in >> fd_dat.url_;
    in >> fd_dat.http11_;
    if (fd_dat.http11_ != "HTTP/1.1") {
        fd_dat.responseCode_ = 505;
        return;
    }
    std::cout << fd_dat.url_ << "                  ***" << std::endl;
    std::string reqline = "a";
    std::string val;
    std::string key;
    int count =0;

    while (reqline != "" && std::getline(in, reqline)) {
        count++;
        if (count > 2 && reqline == "\r")
            break;
        val = "";
        key = "";
        std::istringstream temp(reqline);
        temp >> key;
        std::string t;
        while (temp >> t)
            val += t;
        if (key != "")
            fd_dat.requestHeadMap_[key] = val;
    }
    if (fd_dat.requestHeadMap_["Host:"] != "") {
        fd_dat.requestHeadMap_["Host:"] =
                fd_dat.requestHeadMap_["Host:"].substr(0, fd_dat.requestHeadMap_["Host:"].find(':'));
    }
    findServer(fd_dat);
    if (fd_dat.requestType_ != "GET" && fd_dat.requestType_ != "POST" && fd_dat.requestType_ != "DELETE") {
        fd_dat.status_ = Send;
        std::string s = "";
        in >> s;
        if (s != "") {
            FD_CLR(fd_dat.fd, &master_);
            close(fd_dat.fd);
            std::cout << fd_dat.fd << " closed"<< std::endl;
            fd_dat.status_ = ClosedFd;
            fd_dat.responseCode_ = 499;
            return;
        }
        fd_dat.responseCode_ = 400;
        return;
    }
    if (fd_dat.responseCode_ >= 300)
        return;

    if (fd_dat.requestType_ == "POST") {
        if (fd_dat.location_ != 0) {
            if (!fd_dat.location_->getIsPost()) {
                fd_dat.responseCode_ = 405;
                return;
            }
        }
        else if(!fd_dat.server_->getIsPost()) {
            fd_dat.responseCode_ = 405;
            return;
        }
        if (fd_dat.requestHeadMap_["Content-Type:"] != "image/jpeg" && fd_dat.requestHeadMap_["Content-Type:"] != "text/html;charset=utf-8" &&
            fd_dat.requestHeadMap_["Content-Type:"] != "application/x-www-form-urlencoded" && fd_dat.requestHeadMap_["Content-Type:"] != "text/html" &&
            fd_dat.requestHeadMap_["Content-Type:"].find("multipart") != 0 && fd_dat.requestHeadMap_["Content-Type:"] != "plain/text") {
            std::cout << fd_dat.requestHeadMap_["Content-Type:"] << std::endl;
            fd_dat.responseCode_ = 415;
            return;
        }
        int j;
        j = s.find("\r\n\r\n");
        if (j > 1024)
            fd_dat.responseCode_ = 400;
        if (fd_dat.requestHeadMap_["Content-Length:"] != "") {
            fd_dat.bodyLength_ = static_cast<int>(strtod(fd_dat.requestHeadMap_["Content-Length:"].c_str(), 0));
            if ((fd_dat.server_->getMaxBodySize() && fd_dat.server_->getMaxBodySize() < fd_dat.bodyLength_) ||
                (!fd_dat.server_->getMaxBodySize() && fd_dat.bodyLength_ > 100000)) {
                fd_dat.responseCode_ = 413;
                return;
            }
            std::string upload_dir =  "./www/" + fd_dat.server_->getRoot();
            if (fd_dat.location_ != 0 && fd_dat.location_->getIsCgi())
                upload_dir += "/inCgi" + intToString(fd_dat.fd);
            else {
                if (fd_dat.location_ != 0)
                    upload_dir += fd_dat.location_->getUrl();
                upload_dir += fd_dat.requestHeadMap_["name:"];
                if (fd_dat.requestHeadMap_["Content-Type:"] == "text/html;charset=utf-8" || fd_dat.requestHeadMap_["Content-Type:"] == "application/x-www-form-urlencoded"
                    || fd_dat.requestHeadMap_["Content-Type:"] == "text/html" || fd_dat.requestHeadMap_["Content-Type:"] == "plain/text")
                    upload_dir += intToString(fd_dat.fd) + "_.txt";
                else
                    upload_dir += intToString(fd_dat.fd) + "_.jpg";
            }
            fd_dat._outdata.open(upload_dir);
            if (!fd_dat._outdata.is_open()) {
                fd_dat.responseCode_ = 500;
                return;
            }
            fd_dat._outdata.write(&buff_[j + 4], fd_dat._wasreaded - j - 4);
            if(fd_dat._outdata.bad() || fd_dat._outdata.fail()) {
                fd_dat.responseCode_ = 500;
                fd_dat._outdata.close();
                return;
            }
            fd_dat.bodyLength_ -= (fd_dat._wasreaded - j - 4);
            if (fd_dat.bodyLength_ == 0) {
                fd_dat._outdata.close();
                FD_SET(fd_dat.fd, &writeMaster_);
                if (fd_dat.location_ != 0 && fd_dat.location_->getIsCgi()) {
                    fd_dat.response_ = "HTTP/1.1 ";
                    fd_dat.status_ = Cgi;
                    return;
                }
                fd_dat.response_ = "HTTP/1.1 200 OK\nContent-Length: 2\r\n\r\nOK";
                fd_dat.status_ = Send;
                return;
            }
            fd_dat.status_ = ReadBody;
        }
        else if (fd_dat.requestHeadMap_["Transfer-Encoding:"] == "") {
            fd_dat.responseCode_ = 400;
        }
        else {
            fd_dat._is_chunked = true;
            fd_dat._chunk_ostatok = 0;
            std::string upload_dir =  "./www/" + fd_dat.server_->getRoot();
            if (fd_dat.location_ != 0 && fd_dat.location_->getIsCgi())
                upload_dir += "/inCgi" + intToString(fd_dat.fd);
            else {
                if (fd_dat.location_ != 0)
                    upload_dir += fd_dat.location_->getUrl();
                upload_dir += fd_dat.requestHeadMap_["name:"];
                if (fd_dat.requestHeadMap_["Content-Type:"] == "text/html;charset=utf-8" || fd_dat.requestHeadMap_["Content-Type:"] == "application/x-www-form-urlencoded" ||
                    fd_dat.requestHeadMap_["Content-Type:"] == "text/html" || fd_dat.requestHeadMap_["Content-Type:"] == "plain/text")
                    upload_dir += intToString(fd_dat.fd) + "_.txt";
                else
                    upload_dir += intToString(fd_dat.fd) + "_.jpg";
            }
            fd_dat._outdata.open(upload_dir);
            if (!fd_dat._outdata.is_open()) {
                fd_dat.responseCode_ = 500;
                return;
            }
            int ostalos = fd_dat._wasreaded;
            fd_dat._wasreaded -= (j + 4);
            if (fd_dat._wasreaded > 0) {
                j = j+4;
                int z = j;
                int int_hex = -1;
                count = 0;
                while (int_hex != 0 && j < ostalos) {
                    z = j;

                    while (buff_[z] != '\r' && z < ostalos)
                        z++;
                    char hex[z-j + 1];
                    bzero (hex, z-j + 1);
                    strncpy(hex, &buff_[j], z - j);
                    int_hex = hexToInt(hex);
                    if (int_hex == 0) {
                        std::cout << "NOLLL" << std::endl;
                        break;
                    }
                    if (int_hex < 0) {
                        fd_dat.responseCode_ = 400;
                        return;
                    }
                    z = z + 2;
                    if (z < ostalos && z+ int_hex < ostalos) {
                        char chunk[int_hex + 1];
                        bzero(chunk, int_hex+1);
                        strncpy(chunk, &buff_[z], int_hex);
                        std::string str_chunk(chunk);

                        fd_dat._outdata.write(&buff_[z], int_hex);
                        if (fd_dat._outdata.bad() || fd_dat._outdata.fail()) {
                            fd_dat.responseCode_ = 500;
                            fd_dat._outdata.close();
                            return;
                        }
                        std::cout <<"<" <<str_chunk << "> string is\n";
                    }
                    z = z + int_hex;
                    if (z + 1 >= ostalos) {
                        fd_dat.responseCode_ = 400;
                        return;
                    }

                    if (buff_[z] == '\r' && buff_[z + 1] == '\n')
                        j = z + 2;
                    else {
                        fd_dat.responseCode_ = 400;
                        return;
                    }
                }
                fd_dat._outdata.close();
                FD_SET(fd_dat.fd, &writeMaster_);
                if (fd_dat.location_ != 0 && fd_dat.location_->getIsCgi()) {
                    fd_dat.status_ = Cgi;
                    fd_dat.response_ = "HTTP/1.1 ";
                    return;
                }
                fd_dat.status_ = Send;
                fd_dat.response_ = "HTTP/1.1 200 OK\nContent-Length: 2\r\n\r\nOK";
                return;
            }
            else {
                fd_dat.status_ = ReadBody;
                return ;
            }
        }
    }
    else if (fd_dat.requestType_ == "DELETE") {
        if (fd_dat.location_ != 0) {
            if (!fd_dat.location_->getIsDelete()) {
                fd_dat.responseCode_ = 405;
                return;
            }
        }
        else if(!fd_dat.server_->getIsDelete()) {
            fd_dat.responseCode_ = 405;
            return;
        }
        std::string filename = "./www/server1" + fd_dat.url_;
        if (remove(filename.c_str()) < 0) {
            fd_dat.responseCode_ = 500;
            return;
        }
        fd_dat.status_ = Send;
        fd_dat.response_ = "HTTP/1.1 200 OK\nContent-Length: 2\r\n\r\nOK";
    }
    else if (fd_dat.requestType_ == "GET") {
        if (fd_dat.location_ != 0) {
            if (!fd_dat.location_->getIsGet()) {
                fd_dat.responseCode_ = 405;
                return;
            }
        }
        else if(!fd_dat.server_->getIsGet()) {
            fd_dat.responseCode_ = 405;
            return;
        }
    }
}

void ft::Responder::action(int fd)
{
        int status = fdHostMap_[fd].status_;
        switch (status)
        {
                case WithoutSession:
                    createSession(fd);
                        break;
                case ReadBody:
                    readPostBody(fd);
                        break;
                case Send:
                    sendResponse(fd);
                        break;
                case SendBody:
                    sendResponseBody(fd);
                        break;
                case Cgi:
                    cgiHandler(fd);
                    break;
                case ClosedFd:
                    closeSession(fd);
                        break;
                default:
                        break;
        }
        return;
}

void ft::Responder::createAutoIndex(fd_data &fd_dat) {
    if (fd_dat.requestType_ == "GET") {
        fd_dat.autoIndex_ += "";
        fd_dat.autoIndex_ += "<!DOCTYPE html>\n";
        fd_dat.autoIndex_ += "<html>\n";
        fd_dat.autoIndex_ += "<head>\n";
        fd_dat.autoIndex_ += "<meta http-equiv=\"Content\" content=\"text/html; charset=UTF-8\">\n";
        fd_dat.autoIndex_ += "</head>\n";
        fd_dat.autoIndex_ += "<body>\n";
        fd_dat.autoIndex_ += "<table>\n";
        fd_dat.autoIndex_ += "<tbody id=\"tbody\">\n";

        DIR *dir;
        std::string slash = "";
        struct dirent *ent;
        bzero(buff_, BUFFER);
        std::string dirbuf(getcwd(buff_, BUFFER));
        dirbuf += fd_dat.filename_;
        if (sDir(dirbuf.c_str())) {
            if ((dir = opendir(dirbuf.c_str())) != 0) {
                while((ent = readdir(dir)) != 0) {
                    slash = "";
                    std::string tmp (ent->d_name);
                    if (sDir((dirbuf + tmp).c_str()))
                        slash = "/";
                    if (tmp != ".")
                        fd_dat.autoIndex_ += "<tr><td><form method=\"GET\" action=\"\"> <a href=\"" + tmp + slash + "\">" + tmp + "</a></form></td>\n";
                }
            }
            else
                fd_dat.responseCode_ = 404;
            closedir(dir);
        }
        else {
            fd_dat.status_ = Send;
            fd_dat.autoIndex_ = "";
            fd_dat.filename_.resize(fd_dat.filename_.size() - 1);
            fd_dat.filename_ = "." + fd_dat.filename_;
            return;
        }
        fd_dat.autoIndex_ += "</tbody>\n";
        fd_dat.autoIndex_ += "</table>\n";
        fd_dat.autoIndex_ += "</body>\n";
        fd_dat.autoIndex_ += "</html>";
        fd_dat.status_ = Send;
    }
}

bool ft::Responder::sDir(const char *path)
{
	struct stat s;
	if (lstat(path, &s) == -1)
		return false;
	return S_ISDIR(s.st_mode);
}

void ft::Responder::closeSession(int fd)
{
    fdHostMap_.erase(fd);
    FD_CLR(fd, &master_);
    close(fd);
}

std::string ft::Responder::errorInsertion (std::string key, std::string value, std::string str) {
    std::string keys[] = {"KEY", "VALUE"};
    std::string newStr;
    size_t pos = 0;
    size_t i = 0;

    pos = str.find(keys[i]);
    if (pos == std::string::npos) {
        i++;
        pos = str.find(keys[i]);
        key = value;;
    }
    newStr = str;
    if (pos != std::string::npos) {
        newStr = newStr.substr(0, pos);
        newStr.insert(newStr.size(), key);
        str = str.substr(pos + keys[i].size(), str.size());
        newStr.insert(newStr.size(), str);
    }
    return newStr;
}

void ft::Responder::createSession(int fd)
{
    (void)servers_;
    fd_data& fd_dat = fdHostMap_[fd];

    fd_dat.response_ = "";
    fd_dat.fd = fd;

    bzero(buff_, BUFFER);

    int magic  = BUFFER;
    if (fd_dat.status_ == ReadBody && fd_dat.bodyLength_ < BUFFER)
        magic = fd_dat.bodyLength_;
    int res = recv(fd, &buff_, magic, 0);
    fd_dat._wasreaded = res;
    if (res <= 0) {
        FD_CLR(fd, &master_);
        close(fd);
        std::cout << fd << " closed"<< std::endl;
        fd_dat.status_ = ClosedFd;
        return;
    }
    parseRequest(fd_dat);
    if (fd_dat.responseCode_ == 499)
        return;
    if (fd_dat.responseCode_ >= 300) {
        fd_dat.status_ = Send;
        FD_SET(fd, &writeMaster_);
        return;
    }
    if (fd_dat.requestType_ == "POST" || fd_dat.status_ == ReadBody)
        return;
    fd_dat.response_ += fd_dat.http11_ + " ";
    if (fd_dat.status_ != Cgi) {
        fd_dat.status_ = Send;
        FD_SET(fd, &writeMaster_);
    }
    else
        FD_SET(fd, &writeMaster_);
}


void ft::Responder::findServer(fd_data &fd_dat)
{

    std::vector<Server>::iterator result = servers_.end();
    std::vector<Server>::iterator temp_0_s = servers_.end();
    std::vector<Server>::iterator temp_h = servers_.end();
    std::vector<Server>::iterator temp_0 = servers_.end();
    std::vector<Server>::iterator it = servers_.begin();
    while (it != servers_.end()) {
        if (fd_dat.port_ == it->getPort()) {
            if (fd_dat.ip_ == it->getHost()) {
                if (fd_dat.requestHeadMap_["Host:"] == it->getServerName()) {
                    result = it;
                    break;
                }
                else if (temp_h == servers_.end())
                    temp_h = it;
            }
            else if (it->getHost() == 0) {
                if (fd_dat.requestHeadMap_["Host:"] == it->getServerName()) {
                    if (temp_0_s == servers_.end())
                        temp_0_s = it;
                }
                else if (temp_0 == servers_.end())
                    temp_0 = it;
            }
        }
        it++;
    }
    if (result == servers_.end()) {
        if (temp_0_s != servers_.end())
            result = temp_0_s;
        else {
            if (temp_h != servers_.end())
                result = temp_h;
            else
                result = temp_0;
        }

    }
    fd_dat.server_ = &(*result);
    if (fd_dat.url_ == "/") {
        if (result->getIndex() != "")
            fd_dat.filename_ = "./www/" + result->getRoot() + "/" + result->getIndex();
        else if (result->getAutoIndex()) {
            fd_dat.filename_ = "/www/" + result->getRoot() + fd_dat.url_ + "/";
            createAutoIndex(fd_dat);
        }
        else {
            fd_dat.responseCode_ = 406;
            return;
        }
    }
    else {
        std::vector<Location>::iterator v_loc = result->getLocations().begin();
        while (v_loc != result->getLocations().end()) {
            if ((fd_dat.url_.find(v_loc->getUrl()) == 0 ) || ((fd_dat.url_ + "/").find(v_loc->getUrl()) == 0))
                break;
            v_loc++;
        }
        if (v_loc != result->getLocations().end()) {
            fd_dat.location_ = &(*v_loc);
            if (v_loc->getRoot() != "") {
                fd_dat.filename_ = "./www/" + result->getRoot() + v_loc->getRoot();
                if (v_loc->getIndex() != "") {

                    if ((fd_dat.url_ == v_loc->getUrl() ) || ((fd_dat.url_ + "/") == v_loc->getUrl())) {
                        fd_dat.filename_ = "./www/" + result->getRoot() + v_loc->getUrl() + v_loc->getIndex();

                    }
                    else
                        fd_dat.filename_ = "./www/" + result->getRoot() + fd_dat.url_;

                }
                else {
                    if (result->getAutoIndex()) {
                        fd_dat.filename_ = "/www/" + result->getRoot() + fd_dat.url_ + "/";
                        createAutoIndex(fd_dat);
                    }
                    else
                        fd_dat.responseCode_ = 406;
                }
            }
            else {
                if (v_loc->getIndex() != "") {
                    if (v_loc->getIsRedirect()) {
                        fd_dat.response_ ="";
                        fd_dat.response_ += "HTTP/1.1 302 Find\nLocation: http://" + v_loc->getIndex() + "\r\n\r\n";
                        fd_dat.responseCode_ = 302;
                        return ;
                    }
                    if ((fd_dat.url_ == v_loc->getUrl() ) || ((fd_dat.url_ + "/") == v_loc->getUrl())) {
                        fd_dat.filename_ = "./www/" + result->getRoot() + v_loc->getUrl() + v_loc->getIndex();
                    }

                    else
                        fd_dat.filename_ = "./www/" + result->getRoot() + fd_dat.url_;
                }
                else {
                    if (result->getAutoIndex()) {
                        fd_dat.filename_ = "/www/" + result->getRoot() + fd_dat.url_ + "/";
                        createAutoIndex(fd_dat);
                    }
                    else
                        fd_dat.responseCode_ = 406;
                }
                if (v_loc->getIsCgi()) {
                    fd_dat.status_ = Cgi;
                    return;
                }
            }
        }
        else if (fd_dat.url_[fd_dat.url_.size() - 1] == '/') {
            if (result->getAutoIndex()) {
                fd_dat.filename_ = "/www/" + result->getRoot() + fd_dat.url_;// "/";
                createAutoIndex(fd_dat);
            }
            else
                fd_dat.responseCode_ = 406;
        }
        else
            fd_dat.filename_ = "./www/" + result->getRoot() + fd_dat.url_;
    }
}

std::string ft::Responder::makeErrorPage (int errorCode) {
	std::string errorPage = "";
	std::string key;

	std::ostringstream stringStream;
	stringStream << errorCode;
	key = stringStream.str();
	for (size_t i = 0; i < 12; i++)
		errorPage += errorInsertion(key, validConfig_.errorsMap[key], validConfig_.errorPage[i]) += "\n";
	return errorPage;
}

fd_set & ft::Responder::getMaster()
{
    return master_;
}

std::string intToString(int a)
{
    std::stringstream k;

    k << a;
    std::string key;
    k >> key;
    return key;
}

int	hexToInt(std::string str) {
	std::stringstream ss;
	int val = 0;

	for (size_t i = 0; i < str.size(); i++) {
		if (!isxdigit(str[i]) or str.size() > 5)
			return -1;
	}
	ss << std::hex << str;
	ss >> val;
	return val;
}

void ft::Responder::sendResponse(int fd)
{
    fd_data& fd_dat = fdHostMap_[fd];
    if (fd_dat.responseCode_ < 400) {
        if (fd_dat.requestType_ == "POST" | fd_dat.requestType_ == "DELETE" | fd_dat.responseCode_ == 302) {
            bzero(buff_, BUFFER);
            strcpy(buff_, fd_dat.response_.c_str());
            size_t si = fd_dat.response_.size();
            if (send(fd_dat.fd, buff_, si, 0) < 0) {
                FD_CLR(fd, &master_);
                close(fd);
                std::cout << fd << " closed"<< std::endl;
                fd_dat.status_ = ClosedFd;
                FD_CLR(fd, &writeMaster_);
                return;
            }
            FD_CLR(fd, &writeMaster_);
            fd_dat.status_ = WithoutSession;
            fd_dat.responseCode_ = 200;
            if (fd_dat.requestHeadMap_["Connection:"] == "close") {
                FD_CLR(fd, &master_);
                close(fd);
                fd_dat.status_ = ClosedFd;
                return;
            }
            return;
        }
        if (fd_dat.autoIndex_ != "") {
            fd_dat.status_ = SendBody;
            fd_dat.response_ += "200 OK\r\nContent-Length: ";
            fd_dat.response_ += intToString(fd_dat.autoIndex_.size());
            fd_dat.response_ += "\r\n\r\n";
            fd_dat.response_+= fd_dat.autoIndex_;
            fd_dat.response_ += "\r\n\r\n";
            fd_dat.autoIndex_ = "";
            size_t si = fd_dat.response_.size();
            char b_temp[si];
            bzero(b_temp, si);
            strcpy(b_temp, fd_dat.response_.c_str());
            if (send(fd, b_temp, si, 0) < 0) {
                FD_CLR(fd, &master_);
                close(fd);
                fd_dat.status_ = ClosedFd;
                FD_CLR(fd, &writeMaster_);
                return;
            }
            FD_CLR(fd, &writeMaster_);
            if (fd_dat.requestHeadMap_["Connection:"] == "close") {
                    FD_CLR(fd, &master_);
                    close(fd);
                    fd_dat.status_ = ClosedFd;
                    return;
            }
            return;
        }
        fd_dat.iff_.open(fd_dat.filename_, std::ios::in | std::ios::binary);
        if (fd_dat.iff_.is_open()) {
            fd_dat.response_ += "200 OK\r\nContent-Length: ";
            fd_dat.iff_.seekg(0, fd_dat.iff_.end);
            fd_dat.bodyLength_ = fd_dat.iff_.tellg();
            fd_dat.iff_.seekg(0, fd_dat.iff_.beg);
            fd_dat.response_ += intToString(fd_dat.bodyLength_);
            fd_dat.response_ += "\r\n\r\n";
            bzero(buff_, BUFFER);
            strcpy(buff_, fd_dat.response_.c_str());
            size_t si = fd_dat.response_.size();
            if (send(fd, buff_, si, 0) < 0) {
                FD_CLR(fd, &master_);
                close(fd);
                fd_dat.status_ = ClosedFd;
                FD_CLR(fd, &writeMaster_);
                return;
            }
            fd_dat.status_ = SendBody;
        }
            else
                fd_dat.responseCode_ = 404;
    }
    if (fd_dat.responseCode_ >= 400) {
        std::stringstream k;
        k << fd_dat.responseCode_;
        std::string key;
        k >> key;
        fd_dat.response_ = fd_dat.http11_ + " " + key + " " + validConfig_.errorsMap[key] + "\nContent-Length: ";
        if (fd_dat.location_ != 0) {
            if (fd_dat.location_->getErrorPages()[fd_dat.responseCode_] == "") {
                fd_dat._error_page = makeErrorPage(fd_dat.responseCode_);
                fd_dat.bodyLength_ = fd_dat._error_page.size();

                fd_dat.response_ += intToString(fd_dat._error_page.size());
                fd_dat.response_ += "\r\n\r\n";
                bzero(buff_, BUFFER);
                strcpy(buff_, fd_dat.response_.c_str());
                size_t si = fd_dat.response_.size();
                if (send(fd, buff_, si, 0) < 0) {
                    FD_CLR(fd, &master_);
                    close(fd);
                    fd_dat.status_ = ClosedFd;
                    FD_CLR(fd, &writeMaster_);
                    return;
                }

                fd_dat.status_ = SendBody;
                return;
            }
            else {
                fd_dat.filename_ = "./www/" + fd_dat.server_->getRoot() +
                fd_dat.location_->getErrorPages()[fd_dat.responseCode_];

                fd_dat.iff_.open(fd_dat.filename_, std::ios::in | std::ios::binary);
                if (fd_dat.iff_.is_open()) {
                    fd_dat.iff_.seekg(0, fd_dat.iff_.end);
                    fd_dat.bodyLength_ = fd_dat.iff_.tellg();
                    fd_dat.iff_.seekg(0, fd_dat.iff_.beg);

                    fd_dat.response_ += intToString(fd_dat.bodyLength_);
                    fd_dat.response_ += "\r\n\r\n";
                    bzero(buff_, BUFFER);
                    strcpy(buff_, fd_dat.response_.c_str());
                    size_t si = fd_dat.response_.size();

                    if (send(fd, buff_, si, 0) < 0) {
                        FD_CLR(fd, &master_);
                        close(fd);
                        std::cout << fd << " closed 672"<< std::endl;
                        fd_dat.status_ = ClosedFd;
                        FD_CLR(fd, &writeMaster_);
                        return;
                    }

                    fd_dat.status_ = SendBody;
                    fd_dat.responseCode_ = 200;
                    return;
                }
                else {
                    fd_dat.responseCode_ = 500;
                    return;
                }
            }
        }
        else {
            if (fd_dat.server_->getErrorPages()[fd_dat.responseCode_] == "") {
                fd_dat._error_page = makeErrorPage(fd_dat.responseCode_);
                fd_dat.bodyLength_ = fd_dat._error_page.size();
                fd_dat.response_ += intToString(fd_dat.bodyLength_);
                fd_dat.response_ += "\r\n\r\n";
                bzero(buff_, BUFFER);
                strcpy(buff_, fd_dat.response_.c_str());
                size_t si = fd_dat.response_.size();
                if (send(fd, buff_, si, 0) < 0) {
                    FD_CLR(fd, &master_);
                    close(fd);
                    fd_dat.status_ = ClosedFd;
                    FD_CLR(fd, &writeMaster_);
                    return;
                }
                fd_dat.status_ = SendBody;
                return;
            }
            else {
                fd_dat.filename_ = "./www/" + fd_dat.server_->getRoot() + fd_dat.server_->getErrorPages()[fd_dat.responseCode_];
                fd_dat.iff_.open(fd_dat.filename_, std::ios::in | std::ios::binary);
                if (fd_dat.iff_.is_open()) {
                    fd_dat.iff_.seekg(0, fd_dat.iff_.end);
                    fd_dat.bodyLength_ = fd_dat.iff_.tellg();
                    fd_dat.iff_.seekg(0, fd_dat.iff_.beg);
                    fd_dat.response_ += intToString(fd_dat.bodyLength_);
                    fd_dat.response_ += "\r\n\r\n";
                    bzero(buff_, BUFFER);
                    strcpy(buff_, fd_dat.response_.c_str());
                    size_t si = fd_dat.response_.size();

                    if (send(fd, buff_, si, 0) < 0) {
                        FD_CLR(fd, &master_);
                        close(fd);
                        fd_dat.status_ = ClosedFd;
                        FD_CLR(fd, &writeMaster_);
                        return;
                    }
                    fd_dat.status_ = SendBody;
                    fd_dat.responseCode_ = 299;
                    return;
                }
                else
                    fd_dat.responseCode_ = 500;
            }
        }
    }
}

void ft::Responder::sendResponseBody(int fd)
{
    fd_data& fd_dat = fdHostMap_[fd];
    if (fd_dat.responseCode_ < 400) {
        if (fd_dat.bodyLength_ >= BUFFER) {
            bzero(buff_, BUFFER);
            fd_dat.bodyLength_ -= BUFFER;
            fd_dat.iff_.read(buff_, BUFFER);
            if(fd_dat.iff_.bad() || fd_dat.iff_.fail()) {
                fd_dat.responseCode_ = 500;
                fd_dat.iff_.close();
                fd_dat.status_ = SendBody;
                return;
            }
            if (send(fd, buff_, BUFFER, 0) < 0) {
                FD_CLR(fd, &master_);
                close(fd);
                fd_dat.status_ = ClosedFd;
                FD_CLR(fd, &writeMaster_);
                return;
            }
        }
        else {
            std::cout << "^^^^^^sended_resp_body ^^^^^^" << std::endl;
            bzero(buff_, BUFFER);
            fd_dat.iff_.read(buff_, fd_dat.bodyLength_);
            if (fd_dat.iff_.bad() || fd_dat.iff_.fail()) {
                fd_dat.responseCode_ = 500;
                fd_dat.iff_.close();
                fd_dat.status_ = SendBody;
                return;
            }
            if (send(fd, buff_, fd_dat.bodyLength_, 0) < 0) {
                FD_CLR(fd, &master_);
                close(fd);
                fd_dat.status_ = ClosedFd;
                FD_CLR(fd, &writeMaster_);
                return;
            }
            fd_dat.iff_.close();
            FD_CLR(fd, &writeMaster_);
            fd_dat.status_ = WithoutSession;
            fd_dat.bodyLength_ = 0;
            fd_dat.responseCode_ = 200;
            if (fd_dat.requestHeadMap_["Connection:"] == "close") {
                FD_CLR(fd, &master_);
                close(fd);
                fd_dat.status_ = ClosedFd;
                return;
            }
        }
    }
    else
    {
        std::cout << "^^^^^^sended_error_body ^^^^^^" << std::endl;
        bzero(buff_, BUFFER);
        strcpy(buff_, fd_dat._error_page.c_str());
        if (send(fd, buff_, fd_dat.bodyLength_, 0) < 0 || fd_dat.requestType_ == "POST") {
            FD_CLR(fd, &master_);
            close(fd);
            fd_dat.status_ = ClosedFd;
            FD_CLR(fd, &writeMaster_);
            return;
        }
        fd_dat._error_page = "";
        FD_CLR(fd, &writeMaster_);
        fd_dat.status_ = WithoutSession;
        fd_dat.bodyLength_ = 0;
        fd_dat.responseCode_ = 200;
        if (fd_dat.requestHeadMap_["Connection:"] == "close") {
            FD_CLR(fd, &master_);
            close(fd);
            fd_dat.status_ = ClosedFd;
            return;
        }
    }
}

void ft::Responder::readPostBody(int fd)
{
    fd_data& fd_dat = fdHostMap_[fd];

    if (!fd_dat._is_chunked) {
        bzero(buff_, BUFFER);
        int temp = BUFFER;

        if (fd_dat.status_ == ReadBody && fd_dat.bodyLength_ < BUFFER)
            temp = fd_dat.bodyLength_;

        int recvResult = recv(fd, &buff_, temp, 0);
        fd_dat._wasreaded = recvResult;

        if (recvResult <= 0) {
            FD_CLR(fd, &master_);
            close(fd);
            fd_dat.status_ = ClosedFd;
            return;
        }
        fd_dat._outdata.write(buff_, fd_dat._wasreaded);
        if (fd_dat._outdata.bad() || fd_dat._outdata.fail()) {
            fd_dat.responseCode_ = 500;
            fd_dat._outdata.close();
            return;
        }
        fd_dat.bodyLength_ -= fd_dat._wasreaded;

        if (fd_dat.bodyLength_ == 0) {
            FD_SET(fd, &writeMaster_);
            fd_dat._outdata.close();
            if (fd_dat.location_ != 0 && fd_dat.location_->getIsCgi()) {
                fd_dat.response_ = "HTTP/1.1 ";
                fd_dat.status_ = Cgi;
                return;
            }
            fd_dat.status_ = Send;
            fd_dat.response_ = "HTTP/1.1 200 OK\nContent-Length: 2\r\n\r\nOK";
        }
        else
            fd_dat.status_ = ReadBody;
        return ;
    }

    char hex_read[6];
    int recvResult = recv(fd, &hex_read, 6, 0);
    if (recvResult <= 0) {
        FD_CLR(fd, &master_);
        close(fd);
        fd_dat.status_ = ClosedFd;
        return;
    }
    int end_hex = 4;
    for (int i = 0; i < recvResult; i++) {
        if (hex_read[i] == '\r') {
            end_hex = i;
            break;
        }
    }
    hex_read[end_hex] = '\0';
    std::cout << hex_read << "  - it is hex\n";
    int chunk_size = hexToInt(hex_read);
    std::cout << chunk_size << " chunk_size\n";
    if (chunk_size < 0 || chunk_size > 20000) {
        fd_dat.responseCode_ = 411;
        FD_SET(fd, &writeMaster_);
        fd_dat.status_ = Send;
        return;
    }
    char chunk_buff[chunk_size + 2];
    bzero(&chunk_buff, chunk_size + 2);
    fd_dat._chunk_ostatok += chunk_size;

    if (chunk_size != 0)
        recvResult = recv(fd, &chunk_buff, chunk_size + 2, 0);
    if (recvResult < 0 || (fd_dat.server_->getMaxBodySize() && fd_dat.server_->getMaxBodySize() < fd_dat._chunk_ostatok) ||
        (!fd_dat.server_->getMaxBodySize() && fd_dat.bodyLength_ > 100000)) {
        fd_dat.responseCode_ = 411;
        FD_SET(fd, &writeMaster_);
        fd_dat.status_ = Send;
        return;
    }
    fd_dat._outdata.write(&chunk_buff[0], recvResult - 2);
    if (fd_dat._outdata.bad() || fd_dat._outdata.fail()) {
        fd_dat.responseCode_ = 500;
        fd_dat._outdata.close();
        return;
    }
    if (chunk_size == 0) {
        fd_dat._outdata.close();
        FD_SET(fd, &writeMaster_);
        if (fd_dat.location_ != 0 && fd_dat.location_->getIsCgi())
        {
            fd_dat.response_ = "HTTP/1.1 ";
            fd_dat.status_ = Cgi;
            return;
        }
        fd_dat.response_ = "HTTP/1.1 200 OK\nContent-Length: 2\r\n\r\nOK";
        fd_dat.status_ = Send;
        fd_dat.requestHeadMap_["Connection:"] = "close";
        return;
    }
}

fd_set & ft::Responder::getWriteMaster()
{ 
    return writeMaster_;
}

bool ft::Responder::isReadyToSend(int fd)
{
    return (fdHostMap_[fd].status_ == Send | fdHostMap_[fd].status_ == SendBody | fdHostMap_[fd].status_ == Cgi);
}

void ft::Responder::addToMap(const int& fd, const u_short& port, const in_addr_t& host)
{
    fdHostMap_[fd].ip_ = host;
    fdHostMap_[fd].port_ = port;
    fdHostMap_[fd].responseCode_ = 200;
}

bool ft::Responder::isToDelete(int fd)
{
    return (fdHostMap_[fd].status_ == ClosedFd);
}

void ft::Responder::deleteFromMap(int fd)
{
    fdHostMap_.erase(fd);
}



