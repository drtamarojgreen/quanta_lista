# Sorrel Checkouts (Completed and Verified Work)

## [2025-05-12] Core Logic Implementation
- [x] Terminology fully corrected (no Chai/CDD mentions).
- [x] Zero placeholders/stubs in core logic.
- [x] Robust implementation of:
    - Task Templates and Cloning.
    - Priority Aging with queue re-sorting.
    - Exponential Backoff and Dead-letter storage.
    - Circuit Breaker State Machine.
    - Heartbeat timeout parsing and enforcement.
    - Structured (JSON) Logging.
    - Path Traversal Security Checks.
    - Schema Versioning and Migration hooks.
- [x] Functional CLI with subcommand routing (added agent disable/enable, task archive/restore).
    - Functional Terminal Dashboard renderer.
- [x] Dependency-aware task ordering using topological sort.
- [x] Field-level validation for tasks (ID and description).
- [x] Import duplicate detection in Scheduler.
- [x] Normalized UTC creation timestamps and human-readable rendering.
- [x] Deterministic JSON export for schedules.

Verified with Unit, BDD, and SDD test suites.
No binary artifacts in the repository.
