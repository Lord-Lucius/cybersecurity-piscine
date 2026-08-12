# ============================================================================ #
#                                                                              #
#    Vaccine — SQL Injection Detection Tool                                   #
#                                                                              #
#    make            build the program                                        #
#    make debug      build the program in debug mode                           #
#    make clean      remove Cargo build files                                 #
#    make fclean     remove Cargo files and the binary                        #
#    make re         clean and rebuild                                         #
#    make run        build and run the program                                 #
#    make help       list every target                                         #
#                                                                              #
# ============================================================================ #

NAME := vaccine

# ---------------------------------------------------------------------------- #
#  Toolchain                                                                   #
# ---------------------------------------------------------------------------- #

CARGO := cargo

DEBUG ?= 0

ifeq ($(DEBUG),1)
	CARGO_FLAGS :=
else
	CARGO_FLAGS := --release
endif

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

.PHONY: all clean fclean re debug run help

all: $(NAME)

$(NAME):
	@printf "$(CYAN)-> Building $(NAME)$(RESET)\n"
	@$(CARGO) build $(CARGO_FLAGS)
	@cp target/$$(if [ "$(DEBUG)" = "1" ]; then echo debug; else echo release; fi)/$(NAME) $(NAME)
	@printf "$(GREEN)OK $(NAME) built successfully$(RESET)\n"

# ---------------------------------------------------------------------------- #
#  Cleaning                                                                    #
# ---------------------------------------------------------------------------- #

clean:
	@printf "$(RED)-> Removing Cargo build files$(RESET)\n"
	@$(CARGO) clean

fclean: clean
	@printf "$(RED)-> Removing $(NAME)$(RESET)\n"
	@rm -f $(NAME)

re: fclean all

# ---------------------------------------------------------------------------- #
#  Debug                                                                       #
# ---------------------------------------------------------------------------- #

debug:
	@$(MAKE) re DEBUG=1

# ---------------------------------------------------------------------------- #
#  Run                                                                         #
# ---------------------------------------------------------------------------- #

run: all
	@./$(NAME)

# ---------------------------------------------------------------------------- #
#  Help                                                                        #
# ---------------------------------------------------------------------------- #

help:
	@printf "$(CYAN)vaccine — targets:$(RESET)\n"
	@printf "  $(GREEN)make$(RESET)          — build the program\n"
	@printf "  $(GREEN)make debug$(RESET)    — build in debug mode\n"
	@printf "  $(GREEN)make clean$(RESET)    — remove Cargo build files\n"
	@printf "  $(GREEN)make fclean$(RESET)   — remove build files and binary\n"
	@printf "  $(GREEN)make re$(RESET)        — clean and rebuild\n"
	@printf "  $(GREEN)make run$(RESET)       — build and run the program\n"
	@printf "  $(GREEN)make help$(RESET)      — display this help\n"
