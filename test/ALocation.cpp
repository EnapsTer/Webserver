#include "ALocation.hpp"

ft::ALocation::ALocation() {
    index_ = "";
    root_ = "";
    autoIndex_ = false;
    uploadPath_ = "";
    isDelete_ = false;
    isPost_ = false;
    isGet_ = false;
}

ft::ALocation &ft::ALocation::operator=(const ft::ALocation &other) {
    root_ = other.root_;
    index_ = other.index_;
    uploadPath_ = other.uploadPath_;
    isGet_ = other.isGet_;
    isPost_ = other.isPost_;
    isDelete_ = other.isDelete_;
    autoIndex_ = other.autoIndex_;
    errorPages_ = other.errorPages_;
    return *this;
}

ft::ALocation::~ALocation() {

}

void ft::ALocation::setRoot(const std::string &root) {
    root_ = root;
}

const std::string &ft::ALocation::getRoot() const {
    return root_;
}

void ft::ALocation::setIndex(const std::string &index) {
    index_ = index;
}

const std::string &ft::ALocation::getIndex() const {
    return index_;
}

void ft::ALocation::setUploadPath(const std::string &path) {
    uploadPath_ = path;
}

const std::string &ft::ALocation::getUploadPath() const {
	return uploadPath_;
}

void ft::ALocation::setIsGet(const bool &status) {
    isGet_ = status;
}

const bool &ft::ALocation::getIsGet() const {
    return isGet_;
}

void ft::ALocation::setIsPost(const bool &status) {
    isPost_ = status;
}

const bool &ft::ALocation::getIsPost() const {
    return isPost_;
}

void ft::ALocation::setIsDelete(const bool &status) {
    isDelete_ = status;
}

const bool &ft::ALocation::getIsDelete() const {
    return isDelete_;
}

void ft::ALocation::setAutoIndex(const bool &autoIndex) {
    autoIndex_ = autoIndex;
}

const bool &ft::ALocation::getAutoIndex() const {
	return autoIndex_;
}

void ft::ALocation::setErrorPages(const int& code, const std::string& path) {
    errorPages_[code] = path;
}

std::map<int, std::string> &ft::ALocation::getErrorPages() {
	return errorPages_;
}
