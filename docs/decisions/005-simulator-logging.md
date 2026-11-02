# 005 — Single simulation event loop with one logging worker

**Status:** accepted architecture, November 1, 2026. Queue limits, overflow behavior, and durability policies remain open.

## Context

The simulator uses a single event loop for physics, TCP, and operational state, as recorded in [decision 004](004-simulation-flight-tcp.md). File output, flushing, and other logging operations can consume unpredictable time in that loop.

The developer selected one logging worker thread to separate output operations from simulation execution. This decision applies to simulator logging only; flight and ground logging arrangements are not selected here.

## Decision

### Ownership and execution

Keep one event loop as the owner of physical state, simulation time, actuator application, and the simulator's communication/request lifecycle. Add one worker thread responsible for consuming log records and writing output.

The logging worker does not advance physics, handle the spacecraft TCP connection, apply actuator requests, or change operational simulation state. It is an auxiliary output worker rather than a second simulation execution owner. A separate logging process is not selected for the initial implementation.

### Record handoff

Pass self-contained log records through a bounded, thread-safe handoff. The event loop captures the event information; the worker performs saving/output operations. Do not give the worker unprotected references into mutable simulator objects or temporary message buffers.

Capture event timestamps when the event occurs, before queueing, rather than substituting the eventual write time. Preserve the distinction between simulation time and real elapsed time from [decision 003](003-simulation-and-flight-timing.md). Run identity and event sequence information should support correlation and identification of gaps; the exact record schema remains open.

Record creation and queue submission still consume event-loop time. Keep that path within the loop's bounded-work requirements. The split between producer-side formatting and worker-side formatting, memory allocation, and synchronization mechanisms remains to be designed.

### Bounds and pressure

Bound pending logging storage. Queue capacity, record-size limits, and logging volume/rate controls still need to be selected.

The logging worker must not introduce an indefinite wait for storage or queue space into the simulation event loop. A dedicated thread alone does not establish that property: the submission and overflow policy must preserve it.

No specific drop-oldest, drop-newest, priority/reservation, or bounded-wait policy is accepted yet. If records are lost or output fails, expose that condition through a bounded status/counter mechanism that does not depend solely on successfully writing another log record.

Whether loss of records essential to replay or acceptance evidence invalidates or ends a run remains open. Logging records are not a substitute for the actuator-result retention and acknowledgment mechanism in decision 004.

### Shutdown and durability

Provide coordinated worker shutdown with an explicit drain/flush policy and bounded waiting. Exact shutdown limits and the treatment of undrained records remain open.

Queueing a record does not establish that it was written or durably saved. An abrupt process termination may lose queued records. This decision does not promise crash-proof logging or power-loss durability.

## Alternatives and tradeoffs

- **Synchronous event-loop logging:** simplest ownership and shutdown, but writing and flushing directly consume simulation execution time.
- **One logging worker:** selected to isolate saving operations while keeping the simulator's operational state under one event-loop owner. Adds a thread-safe handoff, pressure policy, error reporting, and shutdown coordination.
- **Separate logging process:** could collect multiple processes' output and survive a simulator-process failure, but adds interprocess transport, lifecycle, and backpressure concerns that are not required initially.

## Verification intent

- Verify that worker records contain captured event-time data and do not read mutable simulator state afterward.
- Slow or block the output sink, then fill the handoff; verify declared memory bounds, the selected overflow policy, and absence of an indefinite event-loop wait.
- Exercise output errors and verify they are observable without recursive/unbounded failure logging.
- Measure producer-side logging cost and simulation pacing under representative logging volume.
- Exercise normal shutdown and a stalled output sink; verify the selected bounded shutdown behavior and accounting for undrained records.

These are intended checks, not implemented tests or passing results.

## Remaining decisions

- Logging library versus a small project-owned implementation; no library is selected.
- Record format, severity levels, event selection, and maximum record size.
- Queue capacity, synchronization, and full-queue policy.
- Evidence-critical versus ordinary diagnostic records and their loss consequences.
- Sink selection, file rotation/retention, buffering, flushing, and error reporting.
- Shutdown timing, worker failure handling, and any future durability requirement.

## Relationship to existing decisions

Decision 004's single event loop remains the sole operational simulator-state owner. This record adds a narrowly scoped logging thread and resolves the logging arrangement left open there. Its earlier avoidance of cross-thread handoffs does not extend to this newly accepted logging handoff.

Only this new record is added; other documents and implementation files are unchanged.

## Revisit triggers

Revisit if logging submission interferes with timing targets, evidence requires stronger durability, or a shared multi-process collector becomes a concrete requirement.
