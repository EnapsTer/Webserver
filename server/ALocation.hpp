#ifndef ALOCATION_HPP
#define ALOCATION_HPP

#include <iostream>
#include <map>

namespace ft {

    class ALocation {

    protected:

        ALocation();
        ALocation &operator=(const ALocation& other);

        std::string index_;
        std::string root_;
		bool autoIndex_;
		std::string uploadPath_;
        bool isDelete_;
        bool isPost_;
        bool isGet_;

		std::map<int, std::string>	errorPages_;

    public:

        virtual ~ALocation();

        const std::string &getIndex() const;
        void setIndex(const std::string &index);

        const std::string &getRoot() const;
        void setRoot(const std::string &root);

		const bool &getAutoIndex() const;
		void setAutoIndex(const bool &autoIndex);

		const std::string &getUploadPath() const;
		void setUploadPath(const std::string &path);

        const bool &getIsDelete() const;
        void setIsDelete(const bool &status);

        const bool &getIsPost() const;
        void setIsPost(const bool &status);

        const bool &getIsGet() const;
        void setIsGet(const bool &status);


		void setErrorPages(const int& code, const std::string& path);
		std::map<int, std::string> &getErrorPages();
    };

}

#endif
