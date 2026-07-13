/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/10 13:03:02 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/13 15:15:27 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_H
# define UTILS_H

#include "inquisitor.h"
#include <stdio.h>
#include <stdlib.h>

/* String */
void usage(char *msg);
void error(char *msg, int error_code, t_config *config);

/* Memory */
void free_ressources(t_config *config);

#endif
