#include "Responder.hpp"


ft::Responder::Responder(std::vector<Server> & vec): _servers(vec), _valid_conf(1)
{
        FD_ZERO(&_master);
        FD_ZERO(&writeMaster);
}

void        ft::Responder::action(int fd)
{
        int status = _fd_host_map[fd]._status;
        switch (status)
        {
                case Nosession:
                        make_session(fd);
                        break;
                case Readbody:
                       read_post_body(fd);
                        break;
                case Send:
                        send_resp(fd);
                        break;
                case Sendbody:
                        send_resp_body(fd);
                        break;
                case Cgi:
                    cgi_handler(fd);
                    break;
                case Closefd:
                        close_session(fd);
                        break;
                default:
                        break;
        }
        return;
}

void        ft::Responder::find_server(fd_data &fd_dat)
{

        std::vector<Server>::iterator result = _servers.end();
	std::vector<Server>::iterator temp_0_s = _servers.end();
	std::vector<Server>::iterator temp_h = _servers.end();
	std::vector<Server>::iterator temp_0 = _servers.end();
	std::vector<Server>::iterator it = _servers.begin();
        while (it != _servers.end())
        {
                if (fd_dat._port == it->getPort())
                {
                        if (fd_dat._ip == it->getHost())
                        {
                                if (fd_dat._request_head_map["Host:"] == it->getServerName())
                                {
                                        result = it;
                                        break;
                                }
                                else if (temp_h == _servers.end())
                                        temp_h = it;
                        }
                        else if (it->getHost() == 0)
                        {
                                if (fd_dat._request_head_map["Host:"] == it->getServerName())
                                {
                                        if (temp_0_s == _servers.end())
                                                temp_0_s = it;
                                }
                                else if (temp_0 == _servers.end())
                                        temp_0 = it;
                        }
                }
                it++;
        }
        if (result == _servers.end())
        {
                if (temp_0_s != _servers.end())
                        result = temp_0_s;
                else
                {
                        if (temp_h != _servers.end())
                                result = temp_h;
                        else
                                result = temp_0;
                }
                
        }
        fd_dat._server = &(*result);
        if (fd_dat._url == "/")
        {
			if (result->getIndex() != "")
                                fd_dat._filename = "./www/" + result->getRoot() + "/" + result->getIndex();
			else if (result->getAutoIndex())
			{
				fd_dat._filename = "/www/" + result->getRoot()  + fd_dat._url + "/";
				make_Autoindex(fd_dat);
			}
			else
			{
				fd_dat._code_resp = 406;
                                return;
			}
        }
        else
        {
			std::vector<Location>::iterator v_loc = result->getLocations().begin();
                while (v_loc != result->getLocations().end())
                {
                        if ((fd_dat._url.find(v_loc->getUrl()) == 0 ) || ((fd_dat._url + "/").find(v_loc->getUrl()) == 0))
                                break;
                        v_loc++;
                }
                if (v_loc != result->getLocations().end())
                {
                        fd_dat._location = &(*v_loc);
                        if (v_loc->getRoot() != "")
                        {
                                fd_dat._filename = "./www/" + result->getRoot() + v_loc->getRoot();
                                if (v_loc->getIndex() != "")
                                {
                                         
                                         if ((fd_dat._url == v_loc->getUrl() ) || ((fd_dat._url + "/") == v_loc->getUrl()))
                                        {
                                                fd_dat._filename = "./www/" + result->getRoot() + v_loc->getUrl() + v_loc->getIndex();
                                        
                                        }
                                        else
                                                fd_dat._filename = "./www/" + result->getRoot() + fd_dat._url;
                                       
                                }
                                else
                                {
                                        if (result->getAutoIndex())
                                        {
                                                fd_dat._filename = "/www/" + result->getRoot() + fd_dat._url + "/";
                                                make_Autoindex(fd_dat);
                                        }
                                        else
                                                fd_dat._code_resp = 406;
                                }
                        }
                        else                                            /*** Lokation without root ***/
                        {
                                if (v_loc->getIndex() != "")            /*** Index is found ***/
                                {
                                        if (v_loc->getIsRedirect())
                                        {
                                                fd_dat._resp ="";
                                                fd_dat._resp += "HTTP/1.1 302 Find\nLocation: http://" + v_loc->getIndex() + "\r\n\r\n";
                                                fd_dat._code_resp = 302;
                                                return ;
                                        }
                                        if ((fd_dat._url == v_loc->getUrl() ) || ((fd_dat._url + "/") == v_loc->getUrl()))
                                        {
                                                fd_dat._filename = "./www/" + result->getRoot() + v_loc->getUrl() + v_loc->getIndex();
                                        
                                        }

                                        else
                                                fd_dat._filename = "./www/" + result->getRoot() + fd_dat._url;
                                
                                }
                        else                                            /*** Lokation without index page ***/
                                {
                                        if (result->getAutoIndex())
                                        {
                                                fd_dat._filename = "/www/" + result->getRoot() + fd_dat._url + "/";
                                                make_Autoindex(fd_dat);
                                        }
                                        else
                                                fd_dat._code_resp = 406;
                                }
                        if (v_loc->getIsCgi())
                        {
                                fd_dat._status = Cgi;
                                return;
                        }
                        }
                }
                else if (fd_dat._url[fd_dat._url.size() - 1] == '/') /*** Location is not found ***/
                {
                        if (result->getAutoIndex())
			{
				fd_dat._filename = "/www/" + result->getRoot()  + fd_dat._url;// "/";
				make_Autoindex(fd_dat);
			}
			else
				fd_dat._code_resp = 406;
                }
                else
                        fd_dat._filename = "./www/" + result->getRoot() + fd_dat._url;
        }
}

void        ft::Responder::parse_request(fd_data &fd_dat)
{
	std::string s(_buff);
	std::istringstream in(s);
        in >> fd_dat._request_type;
        in >> fd_dat._url;
        in >> fd_dat._http11;
        if (fd_dat._http11 != "HTTP/1.1")
        {
                fd_dat._code_resp = 505;
                return;
        }
	std::cout << fd_dat._url << "                  ***"<< std::endl;
	std::string reqline = "a";
	std::string val;
	std::string key;
        int count =0;
        while (reqline != "" && std::getline(in, reqline))
        {
                count++;
                if (count > 2 && reqline == "\r")
                        break;
                val = "";
                key = "";
			std::istringstream temp(reqline);
                temp >> key;
			std::string t;
                while (temp >> t)
                {
                        val += t;
                }
                if (key != "")
                        fd_dat._request_head_map[key] = val;
        }
        if (fd_dat._request_head_map["Host:"] != "")
        {
                fd_dat._request_head_map["Host:"] = 
                        (fd_dat._request_head_map["Host:"]).substr(0, fd_dat._request_head_map["Host:"].find(':'));
        }
        find_server(fd_dat);
            if (fd_dat._request_type != "GET" && fd_dat._request_type != "POST" && fd_dat._request_type != "DELETE")
        {
                fd_dat._status = Send;
                std::string s = "";
                in >> s;
                if (s != "")
                {
                    FD_CLR(fd_dat.fd, &_master);
                    close(fd_dat.fd);
                    std::cout << fd_dat.fd << " closed"<< std::endl;
                    fd_dat._status = Closefd;
                    fd_dat._code_resp = 499;
                    return;
                }
                fd_dat._code_resp = 400;
                return;
        }
        if (fd_dat._code_resp >= 300)
                return;
        
        if (fd_dat._request_type == "POST")
        {
                if (fd_dat._location != 0)
                {
                        if (!fd_dat._location->getIsPost())
                        {
                                fd_dat._code_resp = 405;
                                return;
                        }
                }
                else if(!fd_dat._server->getIsPost())
                {
                        fd_dat._code_resp = 405;
                        return;
                }
                if (fd_dat._request_head_map["Content-Type:"] != "image/jpeg" && fd_dat._request_head_map["Content-Type:"] != "text/html;charset=utf-8" &&
                fd_dat._request_head_map["Content-Type:"] != "application/x-www-form-urlencoded" && fd_dat._request_head_map["Content-Type:"] != "text/html" &&
                fd_dat._request_head_map["Content-Type:"].find("multipart") != 0 && fd_dat._request_head_map["Content-Type:"] != "plain/text")
                {
                    std::cout << fd_dat._request_head_map["Content-Type:"] << std::endl;
                    fd_dat._code_resp = 415;
                        return;
                }
                int j;      
                j = s.find("\r\n\r\n");
                if (j > 1024)
                        fd_dat._code_resp = 400;
                if (fd_dat._request_head_map["Content-Length:"] != "")
                {
                        fd_dat._len_body = static_cast<int>(strtod(fd_dat._request_head_map["Content-Length:"].c_str(), 0));
                        if ((fd_dat._server->getMaxBodySize() && fd_dat._server->getMaxBodySize() < fd_dat._len_body) ||
                                (!fd_dat._server->getMaxBodySize() && fd_dat._len_body > 100000))
                        {
                                fd_dat._code_resp = 413;
                                return;
                        }
                        std::string upload_dir =  "./www/" + fd_dat._server->getRoot();
                        if (fd_dat._location !=0 && fd_dat._location->getIsCgi())
                                upload_dir += "/inCgi" + int_to_string(fd_dat.fd);     
                        else
                        {
                                                if (fd_dat._location != 0)
                                                upload_dir += fd_dat._location->getUrl();
                                        upload_dir += fd_dat._request_head_map["name:"];
                                        if (fd_dat._request_head_map["Content-Type:"] == "text/html;charset=utf-8" || fd_dat._request_head_map["Content-Type:"] == "application/x-www-form-urlencoded"
                                        || fd_dat._request_head_map["Content-Type:"] == "text/html" || fd_dat._request_head_map["Content-Type:"] == "plain/text")
                                                upload_dir += int_to_string(fd_dat.fd) + "_.txt";
                                        else
                                                upload_dir += int_to_string(fd_dat.fd) + "_.jpg";
                        }
                        // fd_dat._filename = upload_dir;
                        fd_dat._outdata.open(upload_dir);
                        if (!fd_dat._outdata.is_open())
                        {
                                fd_dat._code_resp = 500;
                                return;
                        }
                        fd_dat._outdata.write(&_buff[ j + 4], fd_dat._wasreaded - j -4);
						if(fd_dat._outdata.bad() || fd_dat._outdata.fail())
						{
							fd_dat._code_resp = 500;
							fd_dat._outdata.close();
							return;
						}
                        fd_dat._len_body -= (fd_dat._wasreaded - j - 4);
                        if (fd_dat._len_body == 0)
                        {
                                fd_dat._outdata.close();
                                FD_SET(fd_dat.fd, &writeMaster);
                                if (fd_dat._location != 0 && fd_dat._location->getIsCgi())
                                {
                                        fd_dat._resp = "HTTP/1.1 ";
                                        fd_dat._status = Cgi;
                                        return;
                                }
                                fd_dat._resp = "HTTP/1.1 200 OK\nContent-Length: 2\r\n\r\nOK";
                                fd_dat._status = Send;
                                return;
                        }
                        fd_dat._status = Readbody;
                }
                else if (fd_dat._request_head_map["Transfer-Encoding:"] == "")
                {
                        fd_dat._code_resp = 400;
                }
                else                                                                    /*** chunked ***/
                {
                        fd_dat._is_chunked = true;
                        fd_dat._chunk_ostatok = 0;
                        std::string upload_dir =  "./www/" + fd_dat._server->getRoot();
                        if (fd_dat._location !=0 && fd_dat._location->getIsCgi())
                                upload_dir += "/inCgi" + int_to_string(fd_dat.fd);  
                        else
                        {
                                if (fd_dat._location != 0)
                                        upload_dir += fd_dat._location->getUrl();
                                upload_dir += fd_dat._request_head_map["name:"];
                                if (fd_dat._request_head_map["Content-Type:"] == "text/html;charset=utf-8" || fd_dat._request_head_map["Content-Type:"] == "application/x-www-form-urlencoded" ||
                                fd_dat._request_head_map["Content-Type:"] == "text/html" || fd_dat._request_head_map["Content-Type:"] == "plain/text")
                                        upload_dir += int_to_string(fd_dat.fd) + "_.txt";
                                else
                                        upload_dir += int_to_string(fd_dat.fd) + "_.jpg";
                        }
                        // fd_dat._filename = upload_dir;
                        fd_dat._outdata.open(upload_dir);
                        if (!fd_dat._outdata.is_open())
                        {
                                fd_dat._code_resp = 500;
                                return;
                        }
                        int ostalos = fd_dat._wasreaded;
                        fd_dat._wasreaded -= (j + 4);
//					std::count << " FD_REaded " << fd_dat._wasreaded << std::endl;
                        if (fd_dat._wasreaded > 0)
                        {
                                j = j+4;
                                int z = j;
                                int int_hex = -1;
                                count = 0;
                                while (int_hex != 0 && j < ostalos)
                                {
                                        z = j;
//									std::count <<  (_buff[z] == '\r') << " b[zzz]" << std::endl;

                                        while (_buff[z]!= '\r' && z < ostalos)
                                                z++;
                                        char hex[z-j + 1];
                                        bzero (hex, z-j + 1);
                                        strncpy(hex, &_buff[j], z-j);
//									count << hex << "hex\n";
                                        int_hex = hexToInt(hex);
                                        if (int_hex == 0)
                                        {
											std::cout << "NOLLL" << std::endl;
                                                break;
                                        }
                                        if (int_hex < 0)
                                        {
                                                fd_dat._code_resp = 400;
                                                return;
                                        }
//									std::count << int_hex <<"int_hex"<< std::endl;
                                        z = z + 2;
                                        if (z < ostalos && z+ int_hex < ostalos)
                                        {
                                                char chunk[int_hex + 1];
                                                bzero(chunk, int_hex+1);
                                                strncpy(chunk, &_buff[z], int_hex);
                                                std::string str_chunk(chunk);

                                                fd_dat._outdata.write(&_buff[z], int_hex);
												if(fd_dat._outdata.bad() || fd_dat._outdata.fail())
												{
													fd_dat._code_resp = 500;
													fd_dat._outdata.close();
													return;
												}
                                                std::cout <<"<" <<str_chunk << "> string is\n";
                                        }
                                        z = z + int_hex;
                                        if (z + 1 >= ostalos)
                                        {
                                                fd_dat._code_resp = 400;
                                                return;
                                        }
                                        
                                        if (_buff[z] == '\r' && _buff[z+1] == '\n')
                                        {
                                                j = z + 2;
                                        }
                                        else
                                        {
                                                fd_dat._code_resp = 400;
                                                return;
                                        }
                                                
                                }
                                fd_dat._outdata.close();
                                FD_SET(fd_dat.fd, &writeMaster);
                                if (fd_dat._location != 0 && fd_dat._location->getIsCgi())
                                {
                                        fd_dat._resp = "HTTP/1.1 ";
                                        fd_dat._status = Cgi;
                                        return;
                                }
                                fd_dat._resp = "HTTP/1.1 200 OK\nContent-Length: 2\r\n\r\nOK";
                                fd_dat._status = Send;
                                return;
                        }
                        else 
                        {
                                fd_dat._status = Readbody;
                                return ;

                        }
                }
        }
        else if (fd_dat._request_type == "DELETE")
        {
                if (fd_dat._location != 0)
                {
                        if (!fd_dat._location->getIsDelete())
                        {
                                fd_dat._code_resp = 405;
                                return;
                        }
                }
                else if(!fd_dat._server->getIsDelete())
                {
                        fd_dat._code_resp = 405;
                        return;
                }
                std::string filename = "./www/server1" + fd_dat._url;
                if (remove(filename.c_str()) < 0)
                {
                        fd_dat._code_resp = 500;
                        return;
                }
                fd_dat._resp = "HTTP/1.1 200 OK\nContent-Length: 2\r\n\r\nOK";
                fd_dat._status = Send;
        }
        else if (fd_dat._request_type == "GET")
        {
                if (fd_dat._location != 0)
                {
                        if (!fd_dat._location->getIsGet())
                        {
                                fd_dat._code_resp = 405;
                                return;
                        }
                }
                else if(!fd_dat._server->getIsGet())
                {
                        fd_dat._code_resp = 405;
                        return;
                }
        }

}

bool ft::Responder::s_dir(const char *path)
{
	struct stat s;
	if (lstat(path, &s) == -1) {
		return false;
	}
	return S_ISDIR(s.st_mode);
}

void ft::Responder::make_Autoindex(fd_data &fd_dat) {

	if (fd_dat._request_type == "GET") {
				fd_dat._autoIndex += "";
				fd_dat._autoIndex += "<!DOCTYPE html>\n";
				fd_dat._autoIndex += "<html>\n";
				fd_dat._autoIndex += "<head>\n";
				fd_dat._autoIndex += "<meta http-equiv=\"Content\" content=\"text/html; charset=UTF-8\">\n";
				fd_dat._autoIndex += "</head>\n";
				fd_dat._autoIndex += "<body>\n";
				fd_dat._autoIndex += "<table>\n";
				fd_dat._autoIndex += "<tbody id=\"tbody\">\n";

				DIR *dir;
				std::string slash = "";
				struct dirent *ent;
				bzero(_buff, BUFFER);
				std::string dirbuf(getcwd(_buff, BUFFER));
				dirbuf += fd_dat._filename;
				if (s_dir(dirbuf.c_str())) {
					if ((dir = opendir(dirbuf.c_str())) != 0) {

						while((ent = readdir(dir)) != 0) {
							slash = "";
							std::string tmp (ent->d_name);
							if (s_dir((dirbuf + tmp).c_str()))
								slash = "/";
							if (tmp != ".")
							fd_dat._autoIndex += "<tr><td><form method=\"GET\" action=\"\"> <a href=\"" + tmp + slash + "\">" + tmp + "</a></form></td>\n";
						}
					}
                                        else
                                        {
                                                fd_dat._code_resp = 404;
                                        }
					closedir(dir);
				}
				else {
					fd_dat._autoIndex = "";
					fd_dat._status = Send;
					fd_dat._filename.resize(fd_dat._filename.size() - 1);
					fd_dat._filename = "." + fd_dat._filename;
					return;
				}
				fd_dat._autoIndex += "</tbody>\n";
				fd_dat._autoIndex += "</table>\n";
				fd_dat._autoIndex += "</body>\n";
				fd_dat._autoIndex += "</html>";
				fd_dat._status = Send;
	}
}

void        ft::Responder::make_session(int fd)
{
        fd_data& fd_dat = _fd_host_map[fd];
        fd_dat.fd = fd;
        fd_dat._resp = "";
        (void)_servers;
        bzero(_buff, BUFFER);
        int magic  = BUFFER;
        if (fd_dat._status == Readbody && fd_dat._len_body < BUFFER)
                magic = fd_dat._len_body;
        int res = recv(fd, &_buff, magic, 0);
        fd_dat._wasreaded = res;
        if (res <= 0)
        {
                FD_CLR(fd, &_master);
                close(fd);
                std::cout << fd << " closed"<< std::endl;
                fd_dat._status = Closefd;
                return;
        }
        parse_request(fd_dat);
        if (fd_dat._code_resp == 499)
            return;
        if (fd_dat._code_resp >= 300)                                           /*** Error pages send ***/
        {
                fd_dat._status = Send;
                FD_SET(fd, &writeMaster);
                return;
        }
        if (fd_dat._request_type == "POST" || fd_dat._status == Readbody)
                return;
        fd_dat._resp += fd_dat._http11 + " ";
        if (fd_dat._status != Cgi) {
            fd_dat._status = Send;
            FD_SET(fd, &writeMaster);
        }
        else
            FD_SET(fd, &writeMaster);
        return;
}

void        ft::Responder::close_session(int fd)
{
        _fd_host_map.erase(fd);
        FD_CLR(fd, &_master);
        close(fd);
        return;
}


                                                        /***    html error page generator       ***/

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

std::string ft::Responder::makeErrorPage (int errorCode) {
	std::string errorPage = "";
	std::string key;

	std::ostringstream ss;
	ss << errorCode;
	key = ss.str();
	for (size_t i = 0; i < 12; i++) {
		errorPage += errorInsertion(key, _valid_conf.errorsMap[key],  _valid_conf.errorPage[i]) += "\n";
	}
	return errorPage;
}

                                        /***    integer/string support          ***/
                                        
std::string int_to_string(int a)
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


void    ft::Responder::send_resp(int fd)
{
        fd_data& fd_dat = _fd_host_map[fd];
        if (fd_dat._code_resp < 400)
        {
                if (fd_dat._request_type == "POST" | fd_dat._request_type == "DELETE" | fd_dat._code_resp == 302)
                {
                        bzero(_buff, BUFFER);
                        strcpy(_buff, fd_dat._resp.c_str());
                        size_t si = fd_dat._resp.size();
                        if (send(fd_dat.fd, _buff, si, 0) < 0)
                        {
                                FD_CLR(fd, &_master);
                                close(fd);
                                std::cout << fd << " closed"<< std::endl;
                                fd_dat._status = Closefd;
                                FD_CLR(fd, &writeMaster);
                                return;
                        }
                        FD_CLR(fd, &writeMaster);
                        fd_dat._status = Nosession;
                        fd_dat._code_resp = 200;
                        if (fd_dat._request_head_map["Connection:"] == "close")
                        {
                                FD_CLR(fd, &_master);
                                close(fd);
//                                cout << fd << " closed"<< endl;
                                fd_dat._status = Closefd;
                                return;
                        }
                        return;
                }
                if (fd_dat._autoIndex != "")
                {

                        fd_dat._status = Sendbody;
                        fd_dat._resp += "200 OK\r\nContent-Length: ";
                        fd_dat._resp += int_to_string(fd_dat._autoIndex.size());
                        fd_dat._resp += "\r\n\r\n";
                        fd_dat._resp+= fd_dat._autoIndex;
                        fd_dat._resp += "\r\n\r\n";
                        fd_dat._autoIndex = ""; 
                        size_t si = fd_dat._resp.size();
                        char b_temp[si];
                        bzero(b_temp, si);
                        strcpy(b_temp, fd_dat._resp.c_str());
                        if (send(fd, b_temp, si, 0) < 0)
                        {
                                FD_CLR(fd, &_master);
                                close(fd);
//                                cout << fd << " closed"<< endl;
                                fd_dat._status = Closefd;
                                FD_CLR(fd, &writeMaster);
                                return;
                        }
                        FD_CLR(fd, &writeMaster);
                        if (fd_dat._request_head_map["Connection:"] == "close")
                        {
                                FD_CLR(fd, &_master);
                                close(fd);
//                                cout << fd << " closed"<< endl;
                                fd_dat._status = Closefd;
                                return;
                        }
                        return;
                }
			fd_dat._iff.open(fd_dat._filename, std::ios::in | std::ios::binary);
                if (fd_dat._iff.is_open())
                {
                        fd_dat._resp += "200 OK\r\nContent-Length: ";
                        fd_dat._iff.seekg(0, fd_dat._iff.end);
                        fd_dat._len_body = fd_dat._iff.tellg();
                        fd_dat._iff.seekg(0, fd_dat._iff.beg);
                        fd_dat._resp += int_to_string(fd_dat._len_body);
                        fd_dat._resp += "\r\n\r\n";
                        bzero(_buff, BUFFER);  
                        strcpy(_buff, fd_dat._resp.c_str());
                        size_t si = fd_dat._resp.size();
                        if (send(fd, _buff, si, 0) < 0)
                        {
                                FD_CLR(fd, &_master);
                                close(fd);
//                                cout << fd << " closed"<< endl;
                                fd_dat._status = Closefd;
                                FD_CLR(fd, &writeMaster);
                                return;
                        }
                        fd_dat._status = Sendbody;
                }
                else
                {
                        fd_dat._code_resp = 404;
//                        cout << "not in!!!!!!" << endl;
                }
        }
        if (fd_dat._code_resp    >= 400)                        /***    Errors          ***/
        {
                std::stringstream k;
                k << fd_dat._code_resp;
                std::string key;
                k >> key;
                fd_dat._resp = fd_dat._http11 + " " + key + " " + _valid_conf.errorsMap[key] + "\nContent-Length: ";
                if (fd_dat._location != 0)
                {
                                if (fd_dat._location->getErrorPages()[fd_dat._code_resp] == "")
                                {
                                        fd_dat._error_page = makeErrorPage(fd_dat._code_resp);
                                        fd_dat._len_body = fd_dat._error_page.size();
                                       
                                        fd_dat._resp += int_to_string(fd_dat._error_page.size());
                                        fd_dat._resp += "\r\n\r\n";
                                        bzero(_buff, BUFFER);  
                                        strcpy(_buff, fd_dat._resp.c_str());
                                        size_t si = fd_dat._resp.size();
                                        if (send(fd, _buff, si, 0) < 0)
                                        {
                                                FD_CLR(fd, &_master);
                                                close(fd);
//                                                std::cout << fd << " closed"<< endl;
                                                fd_dat._status = Closefd;
                                                FD_CLR(fd, &writeMaster);
                                                return;
                                        }
                                     
                                        fd_dat._status = Sendbody;
                                        return;
                                }
                                else                                /***        call error-html-gen     ***/
                                {
                                        fd_dat._filename = "./www/" + fd_dat._server->getRoot() +
                                        fd_dat._location->getErrorPages()[fd_dat._code_resp];
                                       
                                        fd_dat._iff.open(fd_dat._filename, std::ios::in | std::ios::binary);
                                        if (fd_dat._iff.is_open())
                                        {
                                                fd_dat._iff.seekg(0, fd_dat._iff.end);
                                                fd_dat._len_body = fd_dat._iff.tellg();
                                                fd_dat._iff.seekg(0, fd_dat._iff.beg);
                                                
                                                fd_dat._resp += int_to_string(fd_dat._len_body);
                                                fd_dat._resp += "\r\n\r\n";
                                                bzero(_buff, BUFFER);  
                                                strcpy(_buff, fd_dat._resp.c_str());
                                                size_t si = fd_dat._resp.size();
                                        
                                                if (send(fd, _buff, si, 0) < 0)
                                                {
                                                        FD_CLR(fd, &_master);
                                                        close(fd);
                                                        std::cout << fd << " closed 672"<< std::endl;
                                                        fd_dat._status = Closefd;
                                                        FD_CLR(fd, &writeMaster);
                                                        return;
                                                }
                                        
                                                fd_dat._status = Sendbody;
                                                fd_dat._code_resp = 200;
                                                return;
                                        }
                                        else
                                        {
                                                fd_dat._code_resp = 500;
                                                return;
                                        }
                                }
                }
                else
                {
                        if (fd_dat._server->getErrorPages()[fd_dat._code_resp] == "")
                                {
                                        fd_dat._error_page = makeErrorPage(fd_dat._code_resp);
                                        fd_dat._len_body = fd_dat._error_page.size();
                                        fd_dat._resp += int_to_string(fd_dat._len_body);
                                        fd_dat._resp += "\r\n\r\n";
                                        bzero(_buff, BUFFER);  
                                        strcpy(_buff, fd_dat._resp.c_str());
                                        size_t si = fd_dat._resp.size();
                                        if (send(fd, _buff, si, 0) < 0)
                                        {
                                                FD_CLR(fd, &_master);
                                                close(fd);
//                                                cout << fd << " closed"<< endl;
                                                fd_dat._status = Closefd;
                                                FD_CLR(fd, &writeMaster);
                                                return;
                                        }
                                        fd_dat._status = Sendbody;
                                        return;
                                }
                                else
                                {
                                        fd_dat._filename = "./www/" + fd_dat._server->getRoot() +
                                        fd_dat._server->getErrorPages()[fd_dat._code_resp];
                                        fd_dat._iff.open(fd_dat._filename, std::ios::in | std::ios::binary);
                                        if (fd_dat._iff.is_open())
                                        {
                                                fd_dat._iff.seekg(0, fd_dat._iff.end);
                                                fd_dat._len_body = fd_dat._iff.tellg();
                                                fd_dat._iff.seekg(0, fd_dat._iff.beg);
                                                fd_dat._resp += int_to_string(fd_dat._len_body);
                                                fd_dat._resp += "\r\n\r\n";
                                                bzero(_buff, BUFFER);  
                                                strcpy(_buff, fd_dat._resp.c_str());
                                                size_t si = fd_dat._resp.size();
                                        
                                                if (send(fd, _buff, si, 0) < 0)
                                                {
                                                        FD_CLR(fd, &_master);
                                                        close(fd);
//                                                        cout << fd << " closed"<< endl;
                                                        fd_dat._status = Closefd;
                                                        FD_CLR(fd, &writeMaster);
                                                        return;
                                                }
                                                fd_dat._status = Sendbody;
                                                fd_dat._code_resp = 299;
                                                return;
                                        }
                                        else
                                        {
                                                fd_dat._code_resp = 500;
                                        }
                                }
                }
        }

}

void ft::Responder::send_resp_body(int fd)
{
        fd_data& fd_dat = _fd_host_map[fd];
        if (fd_dat._code_resp < 400)                            /***            Errors          ***/
        {
                if (fd_dat._len_body >= BUFFER)
                {
                        bzero(_buff, BUFFER);
                        fd_dat._len_body -= BUFFER;
                        fd_dat._iff.read(_buff, BUFFER);
						if(fd_dat._iff.bad() || fd_dat._iff.fail())
						{
							fd_dat._code_resp = 500;
							fd_dat._iff.close();
							fd_dat._status = Sendbody;
							return;
						}
                        if (send(fd, _buff, BUFFER, 0) < 0)
                        {
                                FD_CLR(fd, &_master);
                                close(fd);
//                                cout << fd << " closed"<< endl;
                                fd_dat._status = Closefd;
                                FD_CLR(fd, &writeMaster);
                                return;
                        }
                }
                else
                {
                        std::cout << "^^^^^^sended_resp_body ^^^^^^" << std::endl;
                        bzero(_buff, BUFFER);
                        fd_dat._iff.read(_buff, fd_dat._len_body);
						if(fd_dat._iff.bad() || fd_dat._iff.fail())
						{
							fd_dat._code_resp = 500;
							fd_dat._iff.close();
							fd_dat._status = Sendbody;
							return;
						}
                        if (send(fd, _buff, fd_dat._len_body, 0) < 0)
                        {
                                FD_CLR(fd, &_master);
                                close(fd);
//                                cout << fd << " closed"<< endl;
                                fd_dat._status = Closefd;
                                FD_CLR(fd, &writeMaster);
                                return;
                        }
                        fd_dat._iff.close();     
                        FD_CLR(fd, &writeMaster);
                        fd_dat._status = Nosession;
                        fd_dat._len_body = 0;
                        fd_dat._code_resp = 200;
                        if (fd_dat._request_head_map["Connection:"] == "close")
                        {
                                FD_CLR(fd, &_master);
                                close(fd);
//                                cout << fd << " closed"<< endl;
                                fd_dat._status = Closefd;
                                return;
                        }
                }
        }
        else    /*** Error pages ***/
        {
                std::cout << "^^^^^^sended_error_body ^^^^^^" << std::endl;
                bzero(_buff, BUFFER);
                strcpy(_buff, fd_dat._error_page.c_str());
                if (send(fd, _buff, fd_dat._len_body, 0) < 0 || fd_dat._request_type == "POST")
                {
                        FD_CLR(fd, &_master);
                        close(fd);
//                        cout << fd << " closed 832"<< endl;
                        fd_dat._status = Closefd;
                        FD_CLR(fd, &writeMaster);
                        return;
                }
                fd_dat._error_page = "";    
                FD_CLR(fd, &writeMaster);
                fd_dat._status = Nosession;
                fd_dat._len_body = 0;
                fd_dat._code_resp = 200;
                if (fd_dat._request_head_map["Connection:"] == "close")
                {
                        FD_CLR(fd, &_master);
                        close(fd);
//                        cout << fd << " closed"<< endl;
                        fd_dat._status = Closefd;
                        return;
                }
        }
}

fd_set & ft::Responder::getMaster()
{
        return _master;
}

fd_set & ft::Responder::getWriteMaster()
{ 
        return writeMaster;
}

bool ft::Responder::is_ready_to_send(int fd)
{
        return (_fd_host_map[fd]._status == Send | _fd_host_map[fd]._status == Sendbody | _fd_host_map[fd]._status == Cgi);
}
bool ft::Responder::is_ready_to_read_body(int fd)
{
        return (_fd_host_map[fd]._status == Readbody);
}

void ft::Responder::add_to_map(const int& fd, const u_short& port, const in_addr_t& host)
{
        _fd_host_map[fd]._ip = host;
        _fd_host_map[fd]._port = port;
        _fd_host_map[fd]._code_resp = 200;
}

bool ft::Responder::is_to_del(int fd)
{
        return (_fd_host_map[fd]._status == Closefd);
}

void ft::Responder::del_from_map(int fd)
{
        _fd_host_map.erase(fd);
}

void        ft::Responder::read_post_body(int fd)
{
        fd_data& fd_dat = _fd_host_map[fd];
        if (!fd_dat._is_chunked)
        {
                bzero(_buff, BUFFER);
                int magic  = BUFFER;
                if (fd_dat._status == Readbody && fd_dat._len_body < BUFFER && !fd_dat._is_chunked)
                        magic = fd_dat._len_body;
                int res = recv(fd, &_buff, magic, 0);
                fd_dat._wasreaded = res;
                if (res <= 0)
                {
                        FD_CLR(fd, &_master);
                        close(fd);
//                        cout << fd << " closed"<< std::endl;
                        fd_dat._status = Closefd;
                        return;
                }
                if (fd_dat._len_body >= BUFFER)
                {
                        fd_dat._outdata.write(_buff, fd_dat._wasreaded);
						if(fd_dat._outdata.bad() || fd_dat._outdata.fail())
						{
							fd_dat._code_resp = 500;
							fd_dat._outdata.close();
							return;
						}
                        fd_dat._len_body -= fd_dat._wasreaded;
                }
                else
                {
                        fd_dat._outdata.write(_buff, fd_dat._wasreaded);
						if(fd_dat._outdata.bad() || fd_dat._outdata.fail())
						{
							fd_dat._code_resp = 500;
							fd_dat._outdata.close();
							return;
						}
                        fd_dat._len_body -= fd_dat._wasreaded;
                }

                if (fd_dat._len_body == 0)
                {
                         FD_SET(fd, &writeMaster);
                        fd_dat._outdata.close();
                        if (fd_dat._location != 0 && fd_dat._location->getIsCgi())
                        {
                                fd_dat._resp = "HTTP/1.1 ";
                                fd_dat._status = Cgi;
                                return;
                        }
                        fd_dat._status = Send;
                        fd_dat._resp = "HTTP/1.1 200 OK\nContent-Length: 2\r\n\r\nOK";
                }
                else
                {
                        fd_dat._status = Readbody;
                }
                return ;
        }
        /***    Chunked         ***/
        {
                char hex_read[6];
                int res = recv(fd, &hex_read, 6, 0);
                if (res <= 0)
                {
                        FD_CLR(fd, &_master);
                        close(fd);
                        fd_dat._status = Closefd;
                        return;
                }
                int end_hex = 4;
                for (int i = 0; i < res; i++)
                {
                        if (hex_read[i] == '\r')
                        {
                                end_hex = i;
                                break;
                        }
                }
                hex_read[end_hex] = '\0';
                std::cout << hex_read << "  - it iz are hex\n";
                int chunk_size = hexToInt(hex_read);
                std::cout << chunk_size << " chunk_size\n";
                if (chunk_size < 0 || chunk_size > 20000)
                {
                        fd_dat._code_resp = 411;
                        FD_SET(fd, &writeMaster);
                        fd_dat._status = Send;
                        return;
                }
                char chunk_buff[chunk_size + 2];
                bzero(&chunk_buff, chunk_size + 2);
                fd_dat._chunk_ostatok += chunk_size;
                if (chunk_size != 0)
                        res = recv(fd, &chunk_buff, chunk_size + 2, 0);
//                std::cout << fd_dat._server->getMaxBodySize()<< " ostatok: fd_dat._chunk_ostatok) " << fd_dat._chunk_ostatok << std::endl;
            if (res < 0 || (fd_dat._server->getMaxBodySize() && fd_dat._server->getMaxBodySize() < fd_dat._chunk_ostatok) ||
                        (!fd_dat._server->getMaxBodySize() && fd_dat._len_body > 100000))
                {
                        fd_dat._code_resp = 411;
                        FD_SET(fd, &writeMaster);
                        fd_dat._status = Send;
                        return;
                }
                fd_dat._outdata.write(&chunk_buff[0], res-2);
				if(fd_dat._outdata.bad() || fd_dat._outdata.fail())
				{
					fd_dat._code_resp = 500;
					fd_dat._outdata.close();
					return;
				}
                if (chunk_size == 0)
                {
                        fd_dat._outdata.close();
                        FD_SET(fd, &writeMaster);
                        if (fd_dat._location != 0 && fd_dat._location->getIsCgi())
                        {
                                fd_dat._resp = "HTTP/1.1 ";
                                fd_dat._status = Cgi;
                                return;
                        }
                        fd_dat._resp = "HTTP/1.1 200 OK\nContent-Length: 2\r\n\r\nOK";
                        fd_dat._status = Send;
                        fd_dat._request_head_map["Connection:"] = "close";
                        return;
                }
                return;
        }
}

