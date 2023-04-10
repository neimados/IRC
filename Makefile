NAME	= ircserv

SRCS	= srcs/main.cpp 
OBJDIR	= objs/
OBJS	= $(addprefix $(OBJDIR), $(SRCS:.cpp=.o))
DEPENDS	= $(addprefix $(OBJDIR), $(SRCS:.cpp=.d))

CPP		= c++
RM		= rm -rf
FLAGS	= -Wall -Wextra -Werror -std=c++98 -MD

%o: %cpp ${INCLUDE}
	${CPP} ${FLAGS} -c $< -o $@

all: $(NAME)

$(OBJDIR):
		mkdir -p $(OBJDIR)

$(OBJDIR)%.o: %.cpp
		$(CXX) $(FLAGS) -c $< -o $@

$(NAME): $(OBJDIR) $(OBJS) Makefile
	$(CXX) $(OBJS) -o $(NAME)

clean:
	${RM} $(OBJDIR)

fclean: clean
	${RM} ${NAME}

re: fclean all

.PHONY: all clean fclean re
-include $(DEPENDS)