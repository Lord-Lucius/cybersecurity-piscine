# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/18 10:57:16 by luluzuri          #+#    #+#              #
#    Updated: 2026/07/18 10:57:17 by luluzuri         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		= ft_onion

# ── couleurs ────────────────────────────────────────────────────────────────
GREEN		= \033[0;32m
YELLOW		= \033[0;33m
CYAN		= \033[0;36m
RED		= \033[0;31m
RESET		= \033[0m

# ── règles ──────────────────────────────────────────────────────────────────

all: up

up:
	@printf "$(CYAN)⬡  Building $(NAME)...$(RESET)\n"
	@docker compose up --build -d > /dev/null 2>&1
	@printf "$(YELLOW)⬡  Waiting for Tor to bootstrap...$(RESET)\n"
	@sleep 5
	@$(MAKE) -s onion

down:
	@printf "$(YELLOW)⬡  Stopping $(NAME)...$(RESET)\n"
	@docker compose down > /dev/null 2>&1
	@printf "$(GREEN)⬡  Done.$(RESET)\n"

clean: down
	@printf "$(YELLOW)⬡  Removing volumes...$(RESET)\n"
	@docker compose down --volumes > /dev/null 2>&1
	@printf "$(GREEN)⬡  Done.$(RESET)\n"

fclean: clean
	@printf "$(RED)⬡  Removing image...$(RESET)\n"
	@docker rmi -f $$(docker images -q $(NAME)-$(NAME) 2>/dev/null) > /dev/null 2>&1 || true
	@printf "$(GREEN)⬡  Done.$(RESET)\n"

re: fclean up

onion:
	@printf "$(GREEN)⬡  Onion address → $(RESET)"
	@docker exec onion cat /var/lib/tor/hidden_service/hostname 2>/dev/null || \
		printf "$(YELLOW)not ready yet — retry in a few seconds$(RESET)\n"

logs:
	@docker logs -f onion

status:
	@docker exec onion ss -tlnp

.PHONY: all up down clean fclean re onion logs status
