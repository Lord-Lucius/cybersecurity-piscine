//! Injection techniques. Phase 4.
//!
//! `compare.rs`: `similar()`, the robust response comparator, and the `Verdict`
//! type. `signatures.rs`: the `SQL_ERRORS` table. `error_based.rs` /
//! `boolean_based.rs`: the two required techniques. Re-export once filled.

pub mod compare;
pub mod signatures;
pub mod error_based;
pub mod boolean_based;
