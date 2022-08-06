//
// Created by Janelle Kassandra on 3/13/22.
//

#ifndef VOVA_ALOCATION_HPP
#define VOVA_ALOCATION_HPP

#include <iostream>
#include <map>

namespace ft {

    class ALocation {

    protected:

        ALocation();
        ALocation &operator=(const ALocation& other);

        std::string _root;
        std::string _index;
		std::string _uploadPath;
        bool _isGet;
        bool _isPost;
        bool _isDelete;
		bool _autoIndex;
		std::map<int, std::string>	_errorPages;

    public:

        virtual ~ALocation();

        void setRoot(const std::string &root);
        const std::string &getRoot() const;

        void setIndex(const std::string &index);
        const std::string &getIndex() const;

		void setUploadPath(const std::string &path);
		const std::string &getUploadPath() const;

        void setIsGet(const bool &status);
        const bool &getIsGet() const;

        void setIsPost(const bool &status);
        const bool &getIsPost() const;

        void setIsDelete(const bool &status);
        const bool &getIsDelete() const;

		void setAutoIndex(const bool &autoIndex);
		const bool &getAutoIndex() const;

		void setErrorPages(const int& code, const std::string& path);
		std::map<int, std::string> &getErrorPages();
    };

}

#endif //VOVA_ALOCATION_HPP
