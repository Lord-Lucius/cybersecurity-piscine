
# ============================================================================ #
#                                                                              #
#                            inquisitor — Makefile                             #
#                                                                              #
#  Dual-purpose (context auto-detected via /.dockerenv):                       #
#    - On the HOST       -> orchestrate the Docker lab (up / build / run ...)  #
#    - In the CONTAINER  -> compile the inquisitor binary                      #
#                                                                              #
#  Host quickstart:                                                            #
#    make          # build image, start the 3-container lab, compile binary    #
#    make run      # launch inquisitor with the lab defaults (Ctrl-C restores) #
#    make test     # trigger an FTP upload/download from the client            #
#    make down     # stop the lab                                              #
#                                                                              #
#  NOTE: the attacker image (./attacker/Dockerfile) must provide `cc`, `make`  #
#        and libpcap headers (e.g. build-base + libpcap-dev).                  #
# ============================================================================ #

NAME := inquisitor

# ---------------------------------------------------------------------------- #
#  Colors                                                                       #
# ---------------------------------------------------------------------------- #
GREEN  := \033[0;32m
YELLOW := \033[0;33m
CYAN   := \033[0;36m
RED    := \033[0;31m
RESET  := \033[0m

ifneq ($(wildcard /.dockerenv),)
# ══════════════════════════════════════════════════════════════════════════ #
#  IN-CONTAINER  ->  BUILD THE BINARY                                          #
# ══════════════════════════════════════════════════════════════════════════ #

CC       := cc
CFLAGS   := -Wall -Wextra -Werror -std=gnu17 -g3 -pthread
CPPFLAGS := -Iinclude -MMD -MP
LDFLAGS  := -pthread
LDLIBS   := -lpcap

# Debug build: `make DEBUG=1`  (asan + ubsan, no optimization)
ifeq ($(DEBUG),1)
    CFLAGS  += -O0 -fsanitize=address,undefined
    LDFLAGS += -fsanitize=address,undefined
else
    CFLAGS  += -O2
endif

SRC_DIR := src
OBJ_DIR := build

SRCS := $(shell find $(SRC_DIR) -name '*.c' 2>/dev/null)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

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

debug:
	@$(MAKE) re DEBUG=1

help:
	@printf "$(CYAN)inquisitor (build mode) — targets:$(RESET)\n"
	@printf "  $(GREEN)make$(RESET) / make all   — build $(NAME)\n"
	@printf "  $(GREEN)make debug$(RESET)        — build with -O0 + asan + ubsan\n"
	@printf "  $(GREEN)make clean/fclean/re$(RESET)\n"

# auto-generated header dependencies (recompile on .h change)
-include $(DEPS)

else
# ══════════════════════════════════════════════════════════════════════════ #
#  ON HOST  ->  ORCHESTRATE THE DOCKER LAB                                     #
# ══════════════════════════════════════════════════════════════════════════ #

COMPOSE := docker compose

# lab defaults: poison server(.2) <-> client(.3). Override with:
#   make run ARGS="<ip-src> <mac-src> <ip-target> <mac-target>"
ARGS ?= 192.168.0.2 02:42:c0:a8:00:02 192.168.0.3 02:42:c0:a8:00:03

.PHONY: all up build run down re logs shell test clean fclean help

# single command, no user intervention: start the lab AND compile the binary
all: up build

up:
	@mkdir -p data
	@[ -f data/hello.txt ] || printf "inquisitor lab file\n" > data/hello.txt
	@printf "$(CYAN)→ Starting lab (build image + up)$(RESET)\n"
	@$(COMPOSE) up -d --build
	@printf "$(GREEN)✓ lab is up$(RESET)\n"

build:
	@printf "$(CYAN)→ Compiling $(NAME) inside attacker$(RESET)\n"
	@$(COMPOSE) exec attacker make

# run inquisitor in the foreground; Ctrl-C triggers the ARP restore
run:
	@printf "$(CYAN)→ Launching $(NAME) — Ctrl-C restores the ARP tables$(RESET)\n"
	@$(COMPOSE) exec attacker ./$(NAME) $(ARGS)

# generate FTP traffic (STOR + RETR) from the victim while inquisitor sniffs
test:
	@printf "$(CYAN)→ FTP upload/download from client (watch the sniffer)$(RESET)\n"
	@$(COMPOSE) exec client sh -c 'echo "\
open -u test,1234 192.168.0.2; \
put /etc/hostname -o upload_test.txt; \
get hello.txt -o /tmp/dl.txt; \
bye" | lftp'

down:
	@printf "$(RED)→ Tearing the lab down$(RESET)\n"
	@$(COMPOSE) down

re: down all

logs:
	@$(COMPOSE) logs -f

shell:
	@$(COMPOSE) exec attacker sh

clean:
	@$(COMPOSE) exec attacker make clean || true

fclean:
	@printf "$(RED)→ Full clean (containers, volumes, binary)$(RESET)\n"
	@$(COMPOSE) down -v --remove-orphans
	@rm -rf build $(NAME)

help:
	@printf "$(CYAN)inquisitor (host mode) — targets:$(RESET)\n"
	@printf "  $(GREEN)make$(RESET)        — start lab + build binary\n"
	@printf "  $(GREEN)make run$(RESET)    — run inquisitor (ARGS=\"...\" to override)\n"
	@printf "  $(GREEN)make test$(RESET)   — trigger an FTP transfer from the client\n"
	@printf "  $(GREEN)make logs$(RESET)   — follow lab logs\n"
	@printf "  $(GREEN)make shell$(RESET)  — shell into the attacker\n"
	@printf "  $(GREEN)make down$(RESET)   — stop the lab\n"
	@printf "  $(GREEN)make fclean$(RESET) — stop lab + remove volumes + binary\n"

endif
