#include "Location.hpp"

ft::Location::Location() {
    url_ = "";
    pathToBinPy_ = "";
    pathToBinSh_ = "";
    isFolder_ = false;
    isRedirect_ = false;
    isCgi_ = false;
    redirectionCode_ = 302;
}

ft::Location &ft::Location::operator=(const Location &other) {
    ALocation::operator=(other);
    url_ = other.url_;
    pathToBinPy_ = other.pathToBinPy_;
    pathToBinSh_ = other.pathToBinSh_;
    isFolder_ = other.isFolder_;
    isRedirect_ = other.isRedirect_;
    isCgi_ = other.isCgi_;
    redirectionCode_ = other.redirectionCode_;
    errorsPages_.insert(other.errorsPages_.begin(), other.errorsPages_.end());
    return *this;
}

ft::Location::~Location() {

}

const std::string &ft::Location::getPathToBinPy() const {
	return pathToBinPy_;
}

void ft::Location::setBinPathPy(const std::string &path) {
    pathToBinPy_ = path;
}

const std::string &ft::Location::getUrl() const {
    return url_;
}

void ft::Location::setUrl(const std::string &url) {
    url_ = url;
}

const std::string &ft::Location::getPathToBinSh() const {
    return pathToBinSh_;
}

void ft::Location::setBinPathSh(const std::string &path) {
    pathToBinSh_ = path;
}

const bool &ft::Location::getIsFolder() const {
    return isFolder_;
}

void ft::Location::setIsFolder(const bool &status) {
    isFolder_ = status;
}

const bool &ft::Location::getIsRedirect() const {
    return isRedirect_;
}

void ft::Location::setIsRedirect(const bool &status) {
    isRedirect_ = status;
}

const bool &ft::Location::getIsCgi() const {
    return isCgi_;
}

void ft::Location::setIsCgi(const bool &status) {
    isCgi_ = status;
}

const int &ft::Location::getRedirectionCode() const {
    return redirectionCode_;
}

void ft::Location::setRedirectionCode(const int &redirectionCode) {
    redirectionCode_ = redirectionCode;
}
