#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>
#include <vector>
#include <map>
#include "ALocation.hpp"

namespace ft {

    class Location : public ALocation {

    private:

        std::string url_;
		std::string pathToBinPy_;
        std::string pathToBinSh_;
        bool isFolder_;
        bool isRedirect_;
        bool isCgi_;
        int redirectionCode_;
        std::map<int, std::string> errorsPages_;

    public:

        Location();
        Location &operator=(const Location &other);
        virtual ~Location();

        const std::string &getUrl() const;
        void setUrl(const std::string &url);

		const std::string &getPathToBinPy() const;
		void setBinPathPy(const std::string &path);

        const std::string &getPathToBinSh() const;
        void setBinPathSh(const std::string &path);

        const bool &getIsFolder() const;
        void setIsFolder(const bool &status);

        const bool &getIsRedirect() const;
        void setIsRedirect(const bool &status);

        const bool &getIsCgi() const;
        void setIsCgi(const bool &status);

        const int &getRedirectionCode() const;
        void setRedirectionCode(const int &redirectionCode);

    };

}

#endif
