/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.rs                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 17:12:39 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/13 10:40:41 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

pub enum HttpMethod {
    Get,
    Post,
}

pub struct Config {
    url: String,
    method: HttpMethod,
    pub(crate) output: Option<String>,
}
