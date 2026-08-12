//! vaccine — SQL injection scanner (library crate).
//!
//! All the logic lives in this library; `src/main.rs` is a thin launcher that
//! calls into it. This split is what lets `tests/` reach the code from the
//! outside. See `docs/organisation-et-tests.md`.
//!
//! Each `pub mod` below maps to a folder in `src/`. The folders group the code
//! by responsibility; fill the leaf `*.rs` files, this file only wires them.

pub mod error;
pub mod cli;
pub mod url;
pub mod http;
pub mod scanner;
pub mod techniques;
pub mod engine;
pub mod extract;
pub mod report;
