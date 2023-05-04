NAME	= ircserv

SRCSDIR	= srcs/
SRCS	= main.cpp Server.cpp User.cpp Channel.cpp
OBJDIR	= objs/
OBJS	= $(addprefix $(OBJDIR), $(SRCS:.cpp=.o))
DEPENDS	= $(addprefix $(OBJDIR), $(SRCS:.cpp=.d))

CPP		= c++
RM		= rm -rf
FLAGS	= -Wall -Wextra -Werror #-std=c++98 -MD

%o: %cpp ${INCLUDE}
	${CPP} ${FLAGS} -c $< -o $@

all: $(NAME)

$(OBJDIR):
		mkdir -p $(OBJDIR)

$(OBJDIR)%.o: $(SRCSDIR)%.cpp
		$(CPP) $(FLAGS) -c $< -o $@

$(NAME): $(OBJDIR) $(OBJS) Makefile
	$(CPP) $(OBJS) -o $(NAME)

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