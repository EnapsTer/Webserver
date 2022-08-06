SRCS		=	main.cpp \
				Server.cpp \
				Parser.cpp \
				Port.cpp \
				WebServ.cpp \
				Location.cpp \
				ALocation.cpp \
				ServerParameters.cpp \
				Responder.cpp \
				Cgi.cpp

OBJS		=	$(SRCS:.cpp=.o)

DEPS		=	$(OBJS:.o=.d)

NAME		=	webserv

CXX			=	c++

CXXFLAGS	= 	-Wall -Werror -Wextra

CPPFLAGS	=	-MMD

RM			=	rm -f

%.o:		%.cpp Makefile
			$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(NAME):	$(OBJS)
			$(CXX) $(CXXFLAGS) $(OBJS) -o $@

all:		$(NAME)

clean:
			$(RM) $(OBJS) $(DEPS)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re

-include	$(DEPS)
