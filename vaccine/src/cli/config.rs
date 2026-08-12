/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   config.rs                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 17:12:39 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/12 17:21:19 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

pub enum HttpMethod {
    Get,
    Post,
}

pub struct Config {
    url: String,
    method: HttpMethod,
    output: Option<String>,
}
