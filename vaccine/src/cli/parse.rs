/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.rs                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 17:05:47 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/14 18:06:04 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

use crate::cli::config::{Config, HttpMethod};
use crate::error::VaccineError::{self, Usage};

pub fn parse(args: Vec<String>) -> Result<Config, VaccineError> {
    let mut method: HttpMethod = HttpMethod::Get;
    let mut output: Option<String> = None;
    let mut url: Option<String> = None;

    let mut i: usize = 1;
    while i < args.len() {
        let token: &str = &args[i];

        if token == "-h" || token == "--help" {
            return Err(Usage("error".to_string()));
        } else if token == "-X" {
            let next: Option<&str> = args.get(i + 1).map(String::as_str);

            method = match next {
                Some("GET") => HttpMethod::Get,
                Some("POST") => HttpMethod::Post,
                Some(_) => return Err(Usage("unknown method".to_string())),
                None => return Err(Usage("-X requires a method (GET/POST)".to_string())),
            };
            i += 2;
        } else if token == "-o" {
            let next: Option<&str> = args.get(i + 1).map(String::as_str);

            match next {
                Some(filename) => {
                    output = Some(filename.to_string());
                    i += 2;
                },
                None => return Err(Usage("-o requires a filename".to_string())),
            };
        } else if token.starts_with('-') {
            return Err(Usage(format!("unknown option: {token}")));
        } else {
            match url {
                Some(_) => {
                    return Err(Usage("only one URL is allowed".to_string()));
                }
                None => {
                    url = Some(token.to_owned());
                    i += 1;
                }
            }
        }
    }

    let url: String = url.ok_or(Usage("missing target URL".to_string()))?;

    Ok(Config { url, method, output })
}

pub fn print_help()
