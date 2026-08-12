//! HTTP client: send a GET/POST request, return a normalised Response. Phase 3.
//!
//! `client.rs`: the `Client` wrapper over the HTTP crate and `send()`.
//! `response.rs`: the `Response` type (status + body). Re-export once filled.

pub mod client;
pub mod response;
