# ============================================================================ #
#                                 libft — Makefile                             #
#                                                                              #
#    make            build libft.a                                             #
#    make clean      remove object files                                       #
#    make fclean     remove object files + libft.a                             #
#    make re         clean rebuild                                             #
# ============================================================================ #

NAME := libft.a

# ---------------------------------------------------------------------------- #
#  Toolchain & flags                                                           #
# ---------------------------------------------------------------------------- #
CC       := cc
CFLAGS   := -Wall -Wextra -Werror
CPPFLAGS := -Iinclude

AR       := ar rcs
RM       := rm -rf

SRC_DIR  := src
OBJ_DIR  := obj

# ---------------------------------------------------------------------------- #
#  Colors                                                                      #
# ---------------------------------------------------------------------------- #
GREEN  := \033[0;32m
YELLOW := \033[0;33m
CYAN   := \033[0;36m
RED    := \033[0;31m
RESET  := \033[0m

# ---------------------------------------------------------------------------- #
#  Sources (auto-detected)                                                     #
# ---------------------------------------------------------------------------- #
SRCS := $(shell find $(SRC_DIR) -name '*.c' 2>/dev/null)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# ---------------------------------------------------------------------------- #
#  Rules                                                                       #
# ---------------------------------------------------------------------------- #
.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	@printf "$(CYAN)-> Archiving $(NAME)$(RESET)\n"
	@$(AR) $(NAME) $(OBJS)
	@printf "$(GREEN)OK $(NAME) built successfully$(RESET)\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c include/libft.h
	@mkdir -p $(dir $@)
	@printf "$(YELLOW)-> Compiling$(RESET) %s\n" "$<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	@printf "$(RED)-> Removing libft objects$(RESET)\n"
	@$(RM) $(OBJ_DIR)

fclean: clean
	@printf "$(RED)-> Removing $(NAME)$(RESET)\n"
	@$(RM) $(NAME)

re: fclean all
