/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.rs                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 12:20:12 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/12 16:09:10 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

use std::env;
use std::process;

fn error(msg: String, code: i32) {
    eprintln!("[ERROR] {msg}", msg=msg);
    process::exit(code);
}

fn check_args() -> Vec<String> {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        error("Not enought arguments".to_owned(), 1);
    }
    args
}

fn main() {
    println!("Hello, world!");
    let args = check_args();
    println!("Passed args check");
    print!("{:?}", args);
}
