//! Scan orchestration: baseline, then params x techniques. Phase 4.
//!
//! `run.rs` holds `run()`, the loop that drives detection and hands off to
//! extraction. Re-export once filled, e.g. `pub use run::run;`.

pub mod run;
