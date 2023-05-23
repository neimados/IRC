NAME	= ircserv

SRCSDIR	= srcs/
SRCS	= main.cpp Server.cpp User.cpp Channel.cpp Messages.cpp \
			cmd/cmdNick.cpp cmd/cmdPass.cpp cmd/cmdUser.cpp \
			cmd/cmdJoin.cpp cmd/cmdPart.cpp cmd/cmdList.cpp  \
			cmd/cmdQuit.cpp
OBJDIR	= objs/
OBJS	= $(addprefix $(OBJDIR), $(SRCS:.cpp=.o))
DEPENDS	= $(addprefix $(OBJDIR), $(SRCS:.cpp=.d))

CPP		= c++
RM		= rm -rf
FLAGS	= -Wall -Wextra -Werror -std=c++98 -MD

$(shell mkdir -p $(dir $(OBJS)))

$(OBJDIR)%.o: $(SRCSDIR)%.cpp
		$(CPP) $(FLAGS) -c $< -o $@

$(NAME): $(OBJS) Makefile
	$(CPP) $(OBJS) -o $(NAME)

all: $(OBJDIR) $(NAME)

clean:
	${RM} $(OBJDIR) 

fclean: clean
	${RM} ${NAME}

re: fclean all

test: all
	clear
	./$(NAME) 6667 machin

.PHONY: all clean fclean re test
-include $(DEPENDS)
