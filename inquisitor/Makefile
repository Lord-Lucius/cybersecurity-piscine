# ============================================================================ #
#                                                                              #
#                            inquisitor — Makefile                             #
#                                                                              #
# ============================================================================ #

NAME        := inquisitor

# ---------------------------------------------------------------------------- #
#  Compilateur & flags                                                         #
# ---------------------------------------------------------------------------- #
CC          := cc
CFLAGS      := -Wall -Wextra -Werror -std=gnu17 -g3
CPPFLAGS    := -Iinclude -MMD -MP
LDFLAGS     :=
LDLIBS      := -lpcap

# Mode debug : `make DEBUG=1`
ifeq ($(DEBUG),1)
    CFLAGS  += -g3 -O0 -fsanitize=address,undefined
    LDFLAGS += -fsanitize=address,undefined
else
    CFLAGS  += -O2
endif

# ---------------------------------------------------------------------------- #
#  Arborescence                                                                #
# ---------------------------------------------------------------------------- #
SRC_DIR     := src
INC_DIR     := include
OBJ_DIR     := build
DEP_DIR     := $(OBJ_DIR)

# ---------------------------------------------------------------------------- #
#  Sources                                                                     #
# ---------------------------------------------------------------------------- #
SRCS        := $(shell find $(SRC_DIR) -name '*.c' 2>/dev/null)
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)

# ---------------------------------------------------------------------------- #
#  Couleurs                                                                    #
# ---------------------------------------------------------------------------- #
GREEN       := \033[0;32m
YELLOW      := \033[0;33m
CYAN        := \033[0;36m
RED         := \033[0;31m
RESET       := \033[0m

# ---------------------------------------------------------------------------- #
#  Règles principales                                                          #
# ---------------------------------------------------------------------------- #
.PHONY: all clean fclean re debug help

all: $(NAME)

$(NAME): $(OBJS)
	@printf "$(CYAN)→ Linking $(NAME)$(RESET)\n"
	@$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	@printf "$(GREEN)✓ $(NAME) built successfully$(RESET)\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@printf "$(YELLOW)→ Compiling$(RESET) %s\n" "$<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	@printf "$(RED)→ Removing build objects$(RESET)\n"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@printf "$(RED)→ Removing $(NAME)$(RESET)\n"
	@rm -f $(NAME)

re: fclean all

# ---------------------------------------------------------------------------- #
#  Cibles utilitaires                                                          #
# ---------------------------------------------------------------------------- #

debug:
	@$(MAKE) re DEBUG=1

help:
	@printf "$(CYAN)inquisitor — available targets:$(RESET)\n"
	@printf "  $(GREEN)make$(RESET)          — build $(NAME)\n"
	@printf "  $(GREEN)make clean$(RESET)    — remove object files\n"
	@printf "  $(GREEN)make fclean$(RESET)   — remove objects + binary\n"
	@printf "  $(GREEN)make re$(RESET)       — full rebuild\n"
	@printf "  $(GREEN)make debug$(RESET)    — build with -g3 + asan + ubsan\n"
	@printf "  $(GREEN)make help$(RESET)     — show this message\n"

# ---------------------------------------------------------------------------- #
#  Inclusion des dépendances auto-générées (-MMD -MP)                          #
#    → garantit la recompilation en cas de modification d'un .h                #
# ---------------------------------------------------------------------------- #
-include $(DEPS)
