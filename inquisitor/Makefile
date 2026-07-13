# ============================================================================ #
#                            inquisitor — Makefile                             #
#                                                                              #
#  Local build (default) + Docker lab orchestration.                          #
#                                                                              #
#    make            build the binary locally (needs cc + libpcap-dev)         #
#    make debug      local build with asan + ubsan                             #
#    make re         clean rebuild                                             #
#    make help       list every target                                        #
#                                                                              #
#    make up         build image, start the 3-container lab, compile inside    #
#    make run        run inquisitor in the lab (ARGS="..." to override)        #
#    make test       trigger an FTP upload/download from the client            #
#    make down       stop the lab                                              #
# ============================================================================ #

NAME := inquisitor

# ---------------------------------------------------------------------------- #
#  Toolchain & flags                                                           #
# ---------------------------------------------------------------------------- #
CC       := cc
CFLAGS   := -Wall -Wextra -Werror -std=gnu17 -g3 -pthread
LIBFT_DIR = libft
LIBFT     = $(LIBFT_DIR)/libft.a
CPPFLAGS := -Iinclude -I$(LIBFT_DIR)/include -MMD -MP
LDFLAGS  := -pthread
LDLIBS   := -lpcap


DEBUG ?=

# Debug build: `make debug` (or DEBUG=1)
ifeq ($(DEBUG),1)
    CFLAGS  += -O0 -fsanitize=address,undefined
    LDFLAGS += -fsanitize=address,undefined
else
    CFLAGS  += -O2
endif

# ---------------------------------------------------------------------------- #
#  Sources / objects                                                           #
# ---------------------------------------------------------------------------- #
SRC_DIR := src
OBJ_DIR := build

SRCS := $(shell find $(SRC_DIR) -name '*.c' 2>/dev/null)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# ---------------------------------------------------------------------------- #
#  Colors                                                                      #
# ---------------------------------------------------------------------------- #
GREEN  := \033[0;32m
YELLOW := \033[0;33m
CYAN   := \033[0;36m
RED    := \033[0;31m
RESET  := \033[0m

# ============================================================================ #
#  Local build (default)                                                      #
# ============================================================================ #
.PHONY: all clean fclean re debug

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT)
	@printf "$(CYAN)-> Linking $(NAME)$(RESET)\n"
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBFT) $(LDLIBS) -o $@
	@printf "$(GREEN)OK $(NAME) built successfully$(RESET)\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@printf "$(YELLOW)-> Compiling$(RESET) %s\n" "$<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(LIBFT):
	@printf "$(CYAN)-> Building libft$(RESET)\n"
	@$(MAKE) -C $(LIBFT_DIR) > /dev/null 2>&1
	@printf "$(GREEN)OK libft built successfully$(RESET)\n"

clean:
	@printf "$(RED)-> Removing build objects$(RESET)\n"
	@rm -rf $(OBJ_DIR)
	@$(MAKE) -C $(LIBFT_DIR) clean

fclean: clean
	@printf "$(RED)-> Removing $(NAME)$(RESET)\n"
	@rm -f $(NAME)
	@$(MAKE) -C $(LIBFT_DIR) fclean

re: fclean all

debug:
	@$(MAKE) re DEBUG=1

-include $(DEPS)

# ============================================================================ #
#  Docker lab orchestration                                                   #
# ============================================================================ #
COMPOSE := docker compose
# lab defaults: poison server(.2) <-> client(.3)
ARGS ?= 192.168.0.2 02:42:c0:a8:00:02 192.168.0.3 02:42:c0:a8:00:03

.PHONY: up down run test logs shell lab-clean help

# one command, no intervention: start the lab AND compile the binary inside it
up:
	@mkdir -p data
	@[ -f data/hello.txt ] || printf "inquisitor lab file\n" > data/hello.txt
	@printf "$(CYAN)-> Starting lab (build image + up)$(RESET)\n"
	@$(COMPOSE) up -d --build
	@printf "$(CYAN)-> Compiling $(NAME) inside attacker$(RESET)\n"
	@$(COMPOSE) exec attacker make re
	@printf "$(GREEN)OK lab is up and $(NAME) is built$(RESET)\n"

run:
	@printf "$(CYAN)-> Launching $(NAME) — Ctrl-C restores the ARP tables$(RESET)\n"
	@$(COMPOSE) exec attacker ./$(NAME) $(ARGS)

test:
	@printf "$(CYAN)-> FTP upload/download from client (watch the sniffer)$(RESET)\n"
	@$(COMPOSE) exec client sh -c 'echo "\
open -u test,1234 192.168.0.2; \
put /etc/hostname -o upload_test.txt; \
get hello.txt -o /tmp/dl.txt; \
bye" | lftp'

down:
	@printf "$(RED)-> Tearing the lab down$(RESET)\n"
	@$(COMPOSE) down

logs:
	@$(COMPOSE) logs -f

shell:
	@$(COMPOSE) exec attacker sh

lab-clean:
	@printf "$(RED)-> Removing containers + volumes$(RESET)\n"
	@$(COMPOSE) down -v --remove-orphans

help:
	@printf "$(CYAN)inquisitor — targets:$(RESET)\n"
	@printf "  $(GREEN)make$(RESET)          — build the binary locally (cc + libpcap-dev)\n"
	@printf "  $(GREEN)make debug$(RESET)    — local build with asan + ubsan\n"
	@printf "  $(GREEN)make re$(RESET)       — clean local rebuild\n"
	@printf "  $(GREEN)make up$(RESET)       — start the Docker lab + compile inside it\n"
	@printf "  $(GREEN)make run$(RESET)      — run inquisitor (ARGS=\"...\" to override)\n"
	@printf "  $(GREEN)make test$(RESET)     — trigger an FTP transfer from the client\n"
	@printf "  $(GREEN)make logs$(RESET)     — follow lab logs\n"
	@printf "  $(GREEN)make shell$(RESET)    — shell into the attacker\n"
	@printf "  $(GREEN)make down$(RESET)     — stop the lab\n"
	@printf "  $(GREEN)make lab-clean$(RESET)— stop lab + remove volumes\n"
