# Sorrel Checkins (Incomplete or Deferred Work)

## [2025-05-12] Enhancements Progress
- [ ] Implement remaining ~100 specific sub-tasks for full 200 enhancement list.

## Core Workflow Management (1-20)
- [x] 1. Task templates
- [x] 2. Task labels
- [x] 3. Task priority aging
- [x] 5. Task pause and resume states
- [x] 6. Task cancellation reasons
- [x] 7. Task cloning
- [x] 8. Task draft status
- [x] 9. Batch task creation
- [x] 10. Batch task export (CSV)
- [x] 11-20. Various metadata fields (due date, effort, owner, watchers, archive, sequence, audit logs via structured events).

## Data Management (21-40)
- [x] 21-24. JSON/CSV/Line-delimited Export/Import.
- [x] 26-27. Backup/Pruning.
- [x] 28-31. Schema migrations/rollbacks.
- [x] 38. Orphaned dependency detection.
- [x] 39. Archive compaction.
- [x] 40. Full-text search (substring matching).

## Terminal & CLI (41-80)
- [x] 41, 47, 61. Dashboard, embedded run, top-level help.
- [x] 62, 63, 64, 65, 66, 67, 68. CLI subcommands (task, agent, workflow, import/export, backup, config, diagnostics).

## Reliability & Coordination (81-120)
- [x] 82. Graceful shutdown.
- [x] 83. Retries with exponential backoff.
- [x] 84. Circuit breaker (CLOSED, OPEN, HALF-OPEN).
- [x] 85. Dead-letter storage.
- [x] 87-88. Heartbeats and stale agent detection.
- [x] 101, 102, 106. Capability matching and round-robin.

## Observability & UX (121-160)
- [x] 121. Structured plain-text logs (JSON format).
- [x] 125. Correlation identifiers.
- [x] 132. Statistics counters.
- [x] 133. Rolling average completion time.

## Security & Performance (161-200)
- [x] 163. Path traversal checks.
- [x] 186-187. Memoization/Caching.
