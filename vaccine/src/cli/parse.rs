/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.rs                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 17:05:47 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/13 10:47:15 by luluzuri         ###   ########.fr       */
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
        let token: &str = &args[1];

        if token.eq("-h") || token.eq("--help") {
            return Err(Usage("error".to_string()));
        } else if token.eq("-X") {
            let next: Option<&str> = args.get(i + 1).map(String::as_str);
            if next == None {
                return Err(Usage("-X required a method (GET/POST)".to_string()));
            }
            method = match next {
                Some("GET") => HttpMethod::Get,
                Some("POST") => HttpMethod::Post,
                _ => return Err(Usage(String::from("unknown method")))
            };
            i = i + 2;
        } else if token.eq("-o") {
            let next: Option<&str> = args.get(i + 1).map(String::as_str);
            if next == None {
                return Err(Usage("-o requires a filename".to_string()));
            }
            output = Some(next);
            i = i + 2;
        } else if token.starts_with('-') {
            return Err(Usage(String::from(format!("unkown option: {token}"))));
        } else {
            if !url.is_none() {
                return Err(Usage(String::from("only one URL is allowed")));
            }
            url = Some(token);
            i = i + 1;
        }
    }

    if url.is_none() {
        return Err(Usage(String::from("missing target URL")));
    }
    Ok(Config { url, method, output })
}
