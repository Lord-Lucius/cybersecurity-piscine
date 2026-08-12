/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.rs                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: luluzuri <luluzuri@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/12 12:20:12 by luluzuri          #+#    #+#             */
/*   Updated: 2026/08/12 16:18:28 by luluzuri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

use std::env;
use std::process;

fn error(msg: String, code: i32) {
    eprintln!("[ERROR] {msg}", msg=msg);
    process::exit(code);
}

/// Arg count should be between 2 and 6 because can go like:
/// ["./vaccine", "-X", "POST", "-o", "res.txt", "http://h/?id=1"],
/// and the program name is always here
fn check_args() -> Vec<String> {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 && args.len() > 6 {
        error("Bad arguments given".to_owned(), 1);
    }
    args
}

fn main() {
    println!("Hello, world!");
    let args = check_args();
    println!("Passed args check");
    print!("{:?}", args);
}
