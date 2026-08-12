//! Schema and data extraction (UNION-based). Phases 6-7.
//!
//! `union.rs`: column-count discovery and UNION helpers. `schema.rs`:
//! databases/tables/columns. `dump.rs`: row extraction. `mysql.rs` / `sqlite.rs`:
//! the per-engine payloads (candidates for a shared `Extractor` trait). Re-export
//! once filled.

pub mod union;
pub mod schema;
pub mod dump;
pub mod mysql;
pub mod sqlite;
