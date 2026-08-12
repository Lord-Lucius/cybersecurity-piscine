//! Command-line parsing: argv -> Config. Phase 1.
//!
//! Once `config.rs` and `parse.rs` hold their types, add re-exports here so the
//! rest of the crate can write `crate::cli::Config` instead of the full path,
//! e.g. `pub use config::{Config, HttpMethod};` and `pub use parse::parse;`.

pub mod config;
pub mod parse;
