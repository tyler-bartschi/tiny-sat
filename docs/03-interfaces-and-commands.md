# 03 — Interfaces, packets, and command semantics

## Scope

Define contracts before choosing byte offsets or API routes. The contracts specify what information crosses boundaries and what receivers can conclude. You choose encoding, libraries, transport, concurrency, and schema.

The first release supports one spacecraft and one commanding ground service. Browser clients request actions through that service. Multiple concurrent commanding services and distributed operator authorization are extensions.

## Boundary contracts

| Boundary | Information exchanged | Contract to define |
| --- | --- | --- |
| Plant → sensor → flight | Observation value, unit, quality, source time | Validity, update cadence, missing data, reset behavior |
| Flight → plant | Payload/load state and other supported actuator requests | Which flight authority may apply changes and when |
| Scenario → experiment | Initial conditions, disturbances, injection schedules | Version, seed, ordering, distinction from operational commands |
| Flight ↔ ground | Binary telemetry, commands, command results, events | Encoding, integrity, identity, validity, version compatibility |
| Ground service ↔ UI | Current view, freshness, history, command submission/results | Validation, units, update/reconnect semantics, bounded requests |
| Ground service ↔ storage | Telemetry, events, command attempts and outcomes | Identity, timestamps, durability boundary, retention, failure behavior |

A transport adapter should not decide whether a spacecraft is healthy. A browser should not decide whether a payload is permitted in SAFE. These are domain responsibilities even if several roles share a module initially.

## Binary wire contract

Before Phase 3 acceptance, create a protocol document and small golden byte fixtures in the location you chose in Phase 0. A golden fixture gives a known message and the exact expected bytes, allowing independently written endpoints to agree without relying solely on round-trip tests.

Specify these properties; choose the representation yourself:

| Property | Question your wire specification must answer |
| --- | --- |
| Identification | How is this protocol distinguished from unrelated data? |
| Version | Which versions does each endpoint accept, and what happens to others? |
| Message type | How are telemetry, command, result, and event messages distinguished? |
| Length | What is counted, what is the maximum total size, and what happens to truncation or trailing data? |
| Routing | Which spacecraft/run does this message concern? |
| Boot/session identity | How are newly booted endpoints distinguished from old traffic? |
| Sequence | Which stream owns a sequence, when does it reset, and how is wrap handled? |
| Encoding | Endianness, integer widths, signedness, strings, booleans, enum values, numeric representation |
| Integrity | Exact checksum/CRC variant, parameters, coverage, byte order, and reference check vector |
| Time | Clock domain, unit, range, resolution, and association with boot/run identity |
| Payload dictionary | Fields, units, validity flags, permitted ranges, and unknown-field policy |

Use explicit serialization; a native C++ structure's memory representation is not a portable wire contract. A checksum/CRC detects selected accidental corruption patterns; it does not authenticate a sender or prove every possible corruption is detected.

Bound all declared lengths before using them to allocate or index memory. Reject unsupported versions/types, invalid lengths, failed integrity checks, and invalid required payload fields without applying a commanded effect. An undecodable frame need not receive a command rejection: if its identity cannot be trusted, dropping it and incrementing a diagnostic counter is appropriate. A structurally valid command rejected for a domain reason should receive a correlated rejection.

Start with complete status snapshots and one application packet per datagram if using UDP. A suggested total packet cap is 1,024 bytes for the local baseline; validate that your chosen dictionary fits it. This cap is a project limit, not a guarantee about every future network's path MTU. Defer application fragmentation. With TCP, define framing and bounded resynchronization/connection-reset behavior for malformed streams.

### Compatibility and ordering

For the first version, explicitly rejecting unsupported versions is sufficient; backward compatibility with arbitrary future schemas is not required. Keep wire version and database schema version conceptually separate.

Sequence numbers are scoped by spacecraft boot and message stream/type as defined by your contract. A telemetry sequence is not a command ID. Choose a wrap policy: serial-number arithmetic with a bounded comparison window, or a sufficiently large counter with an explicit end-of-session policy. Test the chosen edge behavior.

Reject or quarantine data from a previously retired boot in the same run. A new boot must be established deliberately from a valid startup/status exchange, not inferred from an arbitrary malformed packet. In the local trusted baseline, a valid packet from the configured spacecraft can announce a new boot; once retired, the previous boot must not take control of the current view again. Record this as a session-consistency mechanism, not authentication.

Establish the active run explicitly through configuration or a deliberate experiment transition. Traffic from a different run must not silently replace it. A scenario reset is an explicit new-run boundary; delayed packets from the prior experiment remain historical or are discarded according to policy.

## Telemetry contract

Use a full snapshot at a suggested 1 Hz. A complete snapshot simplifies recovery after missing packets. At minimum it reports:

- Spacecraft identity, run/boot identity, sequence, source simulation time, and uptime within that boot.
- Current mode, mode-entry reason, payload state, and active faults/recovery readiness.
- Battery charge, temperature, their units and quality, and last successful observation times or ages.
- Required subsystem health, including sensor reset counts.
- Task timing/overrun counters and packet/command diagnostic counters, either in this snapshot or a documented lower-rate health message.
- Protocol/software version information sufficient to interpret the run, available through status or startup metadata.

Per-field observation age matters even when a snapshot itself is new. Freshly transmitted old sensor data must remain marked old. Missing or synthetic orientation must not look like measured attitude.

Operational events include mode changes, fault activation/clearing, command rejection/completion, startup, and relevant degraded conditions. A full event retransmission/archive service is optional. Ground must show gaps where appropriate; current-state resynchronization is required, perfect reconstruction of events lost during an outage is not.

## Command inventory

| Command intent | Permitted effect | Important constraints |
| --- | --- | --- |
| Request status | Produce a current full snapshot/result | Read-only; still identified and rate bounded |
| Request SAFE | Disable payload and enter/stay SAFE | No automatic return to NORMAL; report no-op if already SAFE |
| Request NORMAL | Exit SAFE after recovery guards, or remain NORMAL | Reject during unresolved faults or incomplete startup |
| Set payload enabled/disabled | Set an absolute state | Enabling requires NORMAL and current permission; disabling is safe and idempotent |
| Reset selected simulated sensor | Begin one subsystem reset and increment its counter once | Supported sensor only; declare busy/reset completion semantics and expected timeout consequence |

Avoid a generic “set arbitrary state” command. Injecting heat or forcing battery charge is a scenario action, not a normal spacecraft command. A full spacecraft reboot command is optional; the baseline restart scenario may terminate/relaunch the flight process externally.

## Command lifecycle

Ground records the request before sending it or explicitly refuses the request if it cannot satisfy its selected recording policy. It assigns an identity that is not reused within its defined session, along with the target spacecraft boot and a bounded validity condition.

Distinguish these facts:

| State/fact | Meaning |
| --- | --- |
| Created/queued locally | Ground accepted the operator request; the spacecraft has not necessarily seen it |
| Sent | Ground handed an attempt to its transport; receipt is unproven |
| Accepted | Flight validated and admitted the command; the effect may not yet be complete |
| Rejected | Flight declined the command and provided a reason |
| Completed | Flight confirms the command's defined effect occurred |
| Failed | Flight admitted it but could not complete the effect and knows that failure |
| Outcome unknown | Ground lacks sufficient evidence of the final effect |

An implementation may combine accepted and completed in one response for immediate commands. It must preserve their meanings. Define completion for each command: a mode request completes when the mode transition is committed; a reset completes when the reset procedure ends, with sensor health reported separately. A reset command can complete even though subsequent sensor recovery guards have not yet qualified.

Treat “waiting too long” as a ground observation, not proof of spacecraft rejection or failure. A late valid terminal result can resolve an outcome-unknown record. Preserve the timeout and late resolution in history.

Result arrival order must not make the lifecycle move backward: an old accepted response received after completion does not replace the terminal outcome. Repeated results are harmless; contradictory terminal reports are a protocol/consistency error to expose, not silently overwrite. Record per-attempt transport responses separately from the command's historical effect outcome.

### Retries and duplicate delivery

A retry uses the same command identity and immutable semantic payload, including the original validity deadline. A new packet sequence may identify the new transmission attempt. A new operator request gets a new command identity even if its requested effect is the same.

Within the target flight boot, repeating the same command must not repeat a non-idempotent effect such as incrementing the sensor-reset count. Return the known status/result when available. The same identity with a different semantic payload is a conflict and must not execute.

Deduplication has a declared scope and bounded resource cost. Choose a policy that remains safe after cached results are evicted: retain relevant identities until expiration, reject requests below a retained acceptance floor, or use another documented bounded replay policy. Simply forgetting an old identity while it could still be accepted is insufficient. When capacity is exhausted, reject/defer new work explicitly rather than silently dropping protection history.

These rules support at-most-once effects within the declared boot/session and validity policy. They do not guarantee delivery, eventual completion, or exactly-once execution across crashes.

### Expiration and stale actions

Define a spacecraft-enforceable validity condition. Ground-only timeouts cannot prevent a delayed packet from executing later.

One possible design is a target-boot simulation-time deadline derived from recent telemetry. Ground uses the spacecraft's reported time domain, refuses to submit state-changing commands from an already stale view, and does not pretend its own clock is synchronized. A suggested command lifetime is 10 simulated seconds, with a declared maximum future horizon. You may choose another enforceable design and document its assumptions.

Check validity at admission and again before a queued command begins its effect. Once an effect has begun, expiration does not magically undo it. Specify how in-progress work completes or fails. Retries must not extend an old command's validity. Responses to expired duplicates can report known history without executing anything. If historical outcome has been evicted, an expired/replay-rejected attempt must not be represented as proof that the original command never executed.

Do not automatically release an outage backlog of state-changing commands when contact returns. The baseline UI disables new state-changing submissions while the view is stale/offline. Reconciliation and fresh operator intent are required after a long interruption.

### Restart boundaries

Commands target a specific flight boot. The new boot rejects commands targeting the old boot. Old queued commands and old results cannot be silently reinterpreted as requests/results for the new boot.

A ground-service restart creates a new command-origin session unless identity state is durably restored by a design you can verify. Pending records from the previous service become unresolved; do not blindly resend them. Reconcile current spacecraft state and recover known results where supported.

If a flight process crashes between applying an effect and sending a result, the old command may remain outcome unknown. Seeing the payload currently disabled can confirm current state but cannot prove which historical command caused it. Keep historical outcome and observed present state separate.

### Bounds and priority

Declare maximum pending commands, accepted work, retry attempts, retry cadence, result retention, and ingress processing per cycle. A suggested baseline allows at most 16 pending ground commands and at most 3 total transmission attempts per command. Choose spacing compatible with the selected round-trip delay and lifetime; for the default impairment profile, 2-second spacing is a useful starting point.

Define when completed or outcome-unknown commands leave the active pending window while their history remains available under the retention policy. This must not silently restart retries or erase unresolved outcomes. Also bound boot/session retirement metadata; if its capacity can be exhausted, define a safe refusal or explicit new-run policy rather than letting old traffic become current again.

Protective flight actions must not wait behind an unbounded command backlog. A flood of status requests must not starve health checks. Decide the arbitration/priority policy and verify it under load; no particular queue implementation is prescribed.

## Link impairment contract

Support independent uplink and downlink settings for fixed delay, bounded jitter, loss, byte corruption, duplication, reordering, and a scheduled complete outage. Keep a no-impairment profile for basic checks.

Apply corruption after application encoding/integrity generation and before application decoding/integrity verification. If an impairment adapter sends the altered payload in a new UDP datagram, the host computes a valid transport checksum for that altered payload; the application's integrity check is what the scenario then exercises.

Specify the order of impairment operations, the jitter distribution, queue capacity, overflow policy, seed, and time basis. Link delay and outage scheduling should use scenario simulation time for reproducible model checks, with a declared mapping in real-time integrated runs. Ground retry and UI freshness timers continue to use ground monotonic elapsed time.

At least one regression scenario must explicitly drop a chosen command result and duplicate a chosen command; a random loss probability alone does not guarantee those cases occur. A seed helps reproduce random choices but does not remove nondeterministic ordering from concurrent live processes. Record the ordered injection/traffic trace when exact reproduction of an integration failure matters.

## Local exposure

Bind the initial services to loopback and keep the first release a local experiment. A checksum and a boot identifier do not provide authentication or authorization. Adding LAN/cloud access, real radios, or multiple operators requires a separate access and transport-security decision; see [07](07-expansions.md).
