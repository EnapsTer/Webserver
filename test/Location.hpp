//
// Created by Janelle Kassandra on 3/10/22.
//

#ifndef VOVA_LOCATION_HPP
#define VOVA_LOCATION_HPP

#include <iostream>
#include <vector>
#include <map>
#include "ALocation.hpp"

namespace ft {

    class Location : public ALocation {

    private:

        std::string _url;
		std::string _binPathPy;
        std::string _binPathSh;
//		std::string _uploadPath;
        bool _isFolder;
        bool _isRedirect;
        bool _isCgi;
        int _redirectionCode;
        std::map<int, std::string> _errorPages;

    public:

        Location();
        Location &operator=(const Location &other);
        virtual ~Location();

        void setUrl(const std::string &url);
        const std::string &getUrl() const;

		void setBinPathPy(const std::string &path);
		const std::string &getBinPathPy() const;

        void setBinPathSh(const std::string &path);
        const std::string &getBinPathSh() const;

//		void setUploadPath(const std::string &path);
//		const std::string &getUploadPath() const;

        void setIsFolder(const bool &status);
        const bool &getIsFolder() const;

        void setIsRedirect(const bool &status);
        const bool &getIsRedirect() const;

        void setIsCgi(const bool &status);
        const bool &getIsCgi() const;

        void setRedirectionCode(const int &redirectionCode);
        const int &getRedirectionCode() const;

        // std::map<int, std::string> &getErrorPages();
    };

}

#endif //VOVA_LOCATION_HPP
