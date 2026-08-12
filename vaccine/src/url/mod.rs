//! Target URL and its injectable parameters. Phase 2.
//!
//! `model.rs`: the `Target` / `Param` types. `parse.rs`: `parse()` (URL string
//! -> Target) and `with_injected()` (rebuild the URL with one param replaced).
//! Re-export from here once filled, e.g. `pub use model::{Target, Param};`.

pub mod model;
pub mod parse;
