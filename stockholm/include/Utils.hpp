/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 09:39:21 by luluzuri          #+#    #+#             */
/*   Updated: 2026/07/08 13:03:14 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>

#define OPT_SILENT (1 << 0)
#define OPT_REVERSE (1 << 1)
#define OPT_VERSION (1 << 2)
#define OPT_HELP (1 << 3)

#define PROGRAM_VERSION "0.0.1"

namespace utils {

void usage(char *av);

} // namespace utils
