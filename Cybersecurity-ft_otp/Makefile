# ============================================================================ #
#                                                                              #
#                              ft_otp  — Makefile                              #
#                                                                              #
# ============================================================================ #

NAME        := ft_otp

# ---------------------------------------------------------------------------- #
#  Compilateur & flags                                                         #
# ---------------------------------------------------------------------------- #
CC          := cc
CFLAGS      := -Wall -Wextra -Werror -std=gnu11 -g3
CPPFLAGS    := -Iinclude -MMD -MP -D_POSIX_C_SOURCE=200809L -D_DARWIN_C_SOURCE
LDFLAGS     :=
LDLIBS      := -lcrypto

# Mode debug : `make DEBUG=1`
ifeq ($(DEBUG),1)
    CFLAGS  += -g3 -O0 -fsanitize=address,undefined
    LDFLAGS += -fsanitize=address,undefined
else
    CFLAGS  += -O2
endif

# ---------------------------------------------------------------------------- #
#  OpenSSL — détection cross-platform                                          #
#    Linux  : libcrypto dans les chemins standards                            #
#    macOS  : OpenSSL via Homebrew (hors des chemins par défaut)              #
# ---------------------------------------------------------------------------- #
UNAME_S     := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    OPENSSL_PREFIX := $(shell brew --prefix openssl@3 2>/dev/null)
    ifneq ($(OPENSSL_PREFIX),)
        CPPFLAGS += -I$(OPENSSL_PREFIX)/include
        LDFLAGS  += -L$(OPENSSL_PREFIX)/lib
    endif
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
	@printf "$(CYAN)ft_otp — available targets:$(RESET)\n"
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
