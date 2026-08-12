//! Result reporting: accumulate findings, print and archive them. Phase 8.
//!
//! `model.rs`: the `Report` struct filled during the scan. `save.rs`: `save()`
//! (file writing, `-o`) and the stdout rendering. Re-export once filled.

pub mod model;
pub mod save;
