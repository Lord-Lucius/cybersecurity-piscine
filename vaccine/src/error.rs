/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.rs                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 17:15:59 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/12 17:19:22 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#[derive(Debug)]
pub enum VaccineError {
    Usage(String),
    Http(String),
    Io(String),
    Parse(String),
}

use std::fmt;

impl fmt::Display for VaccineError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            VaccineError::Usage(m) => write!(f, "usage: {m}"),
            VaccineError::Http(m) => write!(f, "http: {m}"),
            VaccineError::Io(m) => write!(f, "io: {m}"),
            VaccineError::Parse(m) => write!(f, "parse: {m}"),
        }
    }
}
