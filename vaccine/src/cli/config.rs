/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.rs                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 17:12:39 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/14 17:57:30 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

pub enum HttpMethod {
    Get,
    Post,
}

pub struct Config {
    pub(crate) url: String,
    pub(crate) method: HttpMethod,
    pub(crate) output: Option<String>,
}
