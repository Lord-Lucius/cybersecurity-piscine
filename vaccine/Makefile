# ============================================================================ #
#                                                                              #
#    Vaccine — SQL Injection Detection Tool                                    #
#                                                                              #
#    make            build the program (release)                              #
#    make debug      build the program in debug mode                          #
#    make run        build then run — pass args with ARGS="..."               #
#    make test       run the test suite (cargo test)                          #
#    make fmt        format the code (cargo fmt)                              #
#    make clippy     run the linter (cargo clippy)                            #
#    make check      fmt-check + clippy + test, the pre-commit gate           #
#    make clean      remove Cargo build files                                 #
#    make fclean     remove Cargo build files and the binary                  #
#    make re         fclean then rebuild                                      #
#    make help       list every target                                       #
#                                                                              #
# ============================================================================ #

NAME := vaccine

# ---------------------------------------------------------------------------- #
#  Toolchain                                                                    #
# ---------------------------------------------------------------------------- #

CARGO := cargo

# DEBUG=1 switches from the optimised release build to the debug build.
DEBUG ?= 0

ifeq ($(DEBUG),1)
	CARGO_FLAGS :=
	PROFILE_DIR := debug
else
	CARGO_FLAGS := --release
	PROFILE_DIR := release
endif

# Arguments forwarded to the program by `make run`.
# Example: make run ARGS='-X POST "http://localhost/login.php"'
ARGS ?=

# ---------------------------------------------------------------------------- #
#  Colors                                                                       #
# ---------------------------------------------------------------------------- #

GREEN  := \033[0;32m
YELLOW := \033[0;33m
CYAN   := \033[0;36m
RED    := \033[0;31m
RESET  := \033[0m

# ============================================================================ #
#  Build (default)                                                             #
# ============================================================================ #

.PHONY: all clean fclean re debug run test test-net fmt fmt-check clippy check help

all: $(NAME)

# Rust sources: any change to one of these makes the binary out of date.
SRCS := Cargo.toml $(shell find src -name '*.rs' 2>/dev/null)

# The subject requires the binary to sit at the project root under the name
# `vaccine`. Cargo drops it in target/<profile>/, so we copy it up.
# Cargo does its own incremental tracking; listing SRCS just lets make skip
# the copy when nothing changed.
$(NAME): $(SRCS)
	@printf "$(CYAN)-> Building $(NAME) ($(PROFILE_DIR))$(RESET)\n"
	@$(CARGO) build $(CARGO_FLAGS)
	@cp target/$(PROFILE_DIR)/$(NAME) $(NAME)
	@printf "$(GREEN)OK $(NAME) built successfully$(RESET)\n"

# ---------------------------------------------------------------------------- #
#  Debug                                                                        #
# ---------------------------------------------------------------------------- #

debug:
	@$(MAKE) re DEBUG=1

# ---------------------------------------------------------------------------- #
#  Run                                                                          #
# ---------------------------------------------------------------------------- #

run: all
	@printf "$(CYAN)-> Running ./$(NAME) $(ARGS)$(RESET)\n"
	@./$(NAME) $(ARGS)

# ---------------------------------------------------------------------------- #
#  Quality gates                                                                #
# ---------------------------------------------------------------------------- #

test:
	@printf "$(CYAN)-> Running tests (offline)$(RESET)\n"
	@$(CARGO) test

# Network tests marked #[ignore]: run only once the local lab is up.
test-net:
	@printf "$(CYAN)-> Running network tests (needs the local lab)$(RESET)\n"
	@$(CARGO) test -- --ignored

fmt:
	@printf "$(CYAN)-> Formatting$(RESET)\n"
	@$(CARGO) fmt

# Non-mutating check used by `make check` and CI: fails if code is unformatted.
fmt-check:
	@$(CARGO) fmt --check

clippy:
	@printf "$(CYAN)-> Linting (clippy)$(RESET)\n"
	@$(CARGO) clippy -- -D warnings

# The gate to pass before every commit.
check: fmt-check clippy test
	@printf "$(GREEN)OK all checks passed$(RESET)\n"

# ---------------------------------------------------------------------------- #
#  Cleaning                                                                     #
# ---------------------------------------------------------------------------- #

clean:
	@printf "$(RED)-> Removing Cargo build files$(RESET)\n"
	@$(CARGO) clean

fclean: clean
	@printf "$(RED)-> Removing $(NAME)$(RESET)\n"
	@rm -f $(NAME)

re: fclean all

# ---------------------------------------------------------------------------- #
#  Help                                                                         #
# ---------------------------------------------------------------------------- #

help:
	@printf "$(CYAN)vaccine — targets:$(RESET)\n"
	@printf "  $(GREEN)make$(RESET)            — build the program (release)\n"
	@printf "  $(GREEN)make debug$(RESET)      — build in debug mode\n"
	@printf "  $(GREEN)make run$(RESET)         — build then run; args via ARGS=\"...\"\n"
	@printf "  $(GREEN)make test$(RESET)       — run the test suite (offline)\n"
	@printf "  $(GREEN)make test-net$(RESET)   — run network tests (needs the lab)\n"
	@printf "  $(GREEN)make fmt$(RESET)        — format the code\n"
	@printf "  $(GREEN)make clippy$(RESET)     — run the linter\n"
	@printf "  $(GREEN)make check$(RESET)      — fmt-check + clippy + test\n"
	@printf "  $(GREEN)make clean$(RESET)      — remove Cargo build files\n"
	@printf "  $(GREEN)make fclean$(RESET)     — remove build files and binary\n"
	@printf "  $(GREEN)make re$(RESET)          — clean and rebuild\n"
	@printf "  $(GREEN)make help$(RESET)        — display this help\n"
	@printf "\n$(YELLOW)example:$(RESET) make run ARGS='\"http://localhost/vulnerable.php?id=1\"'\n"
