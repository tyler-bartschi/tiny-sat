# 04 — Ground-control behavior and operator experience

## Responsibility

Ground control turns received messages into an explainable operational view and traceable command requests. It does not own spacecraft truth or override flight protection. The ground service is the operational source for the browser; the spacecraft remains the authority on its mode and executed effects.

The first UI can be small. Favor a reliable status view, a few trends, and a clear command timeline over a visually elaborate mission-control display.

## Required views

| View | Minimum useful content |
| --- | --- |
| Current spacecraft state | Mode/reason, payload state, battery, temperature, sensor quality, boot identity, active faults, recovery readiness |
| Contact/freshness | Ground-service connection, last advancing spacecraft telemetry receipt, source timestamp, local receipt age, stale/offline state |
| Trends | Battery and temperature over a bounded time window, units, missing/invalid intervals, boot/run discontinuities |
| Commands | Operator intent, identity, target boot, attempts, accepted/rejected/completed/failed/unknown outcome, reasons and timestamps |
| Events | Mode transitions, faults/clears, restarts, link changes known to ground, storage/processing degradation |

Show simulation time alongside a human-readable receipt time where helpful. Label each time rather than using an unexplained “timestamp.” Show orientation only if implemented, with its declared quality.

## Freshness and connectivity

The suggested 1 Hz telemetry profile uses these adjustable local elapsed-time thresholds:

| State | Suggested rule | Operator meaning |
| --- | --- | --- |
| Waiting | No accepted current-boot snapshot yet | Spacecraft state is unknown |
| Current | Less than 3 s since an advancing valid snapshot was received | Recently received view, subject to source-time and field-quality limits |
| Stale | At least 3 s and less than 10 s without advancement | Preserve last values but mark them stale |
| Offline/timeout | At least 10 s without advancement | No recent downlink evidence; do not infer spacecraft power loss |

“Advancing” means newer in the current boot according to the contract. Repeated or out-of-order old snapshots must not keep the current indicator green. Field quality and field source age still apply to an otherwise current packet.

These thresholds measure time since receipt of progress. A deliberately delayed link can deliver a recently received snapshot whose source state is old. If the system has a trusted source-to-ground clock mapping, also show source age. Without one, show the spacecraft source time, receipt age, and known configured delay separately; do not label receipt age as exact measurement age.

A downlink indicator says nothing conclusive about uplink health. A failed command/result exchange can reveal trouble but does not identify which direction failed without more evidence. Display directional impairment settings only as scenario information, clearly distinguished from inferred operational status.

Browser-to-service connectivity is separate. When the browser loses its backend connection, show that promptly and age its cached data locally. A frozen browser display must not remain apparently live because the last received backend status was “current.” A suggested UI connection-loss target is 3 elapsed seconds when detectable by the selected heartbeat/update mechanism.

## Commands from the operator

Use controls for the small command inventory in [03](03-interfaces-and-commands.md). Present the requested action and its constraints in ordinary language. For example, a NORMAL request should show why recovery is blocked, and a sensor reset should explain that observations will temporarily stop.

The service validates request shape and supported values, assigns identity, records attempts, and presents spacecraft results. Flight performs authoritative state validation again. A disabled UI button is helpful guidance but is not the enforcement mechanism.

For the baseline, disable state-changing submission when telemetry is stale/offline, startup state is unknown, or the backend is disconnected. Status requests may still be allowed and rate bounded. After service or spacecraft restart, obtain a fresh status view before enabling state-changing controls.

Do not optimistically label spacecraft mode or payload state as changed after a click. Show the pending intent separately until a result or fresh telemetry provides the appropriate evidence. A command can be accepted yet unfinished. A timeout is shown as outcome unknown, with the last known stage and an explanation of available evidence.

Retries preserve the original command identity. An explicit new operator action after reconciliation gets a new identity and should explain any uncertainty about a previous non-idempotent action. Do not hide failed attempts or replace the timeline with only the latest result.

## Persistence and history

Choose the database and schema yourself. Preserve enough information to explain a run:

- Run, spacecraft, boot, protocol/software versions, scenario/configuration reference, and relevant random seeds.
- Accepted telemetry with source time, ground receipt time, sequence, values, and quality.
- Command intent, semantic payload, identity, target boot, local state, each attempt, and correlated results.
- Operational events, fault/mode changes, decode/error summaries, and ground degraded-state events.

Raw packets are optional, useful diagnostic evidence. If retained, keep them bounded and distinguish decode failures from valid telemetry. Do not insert malformed data into the normal telemetry table as zeros.

Define a stable identity for duplicate records and a policy for storing late snapshots. You may preserve late data historically while refusing to replace the current view. The schema must distinguish source order from receipt order and preserve boot boundaries.

Provide a bounded historical query and at least one export format, such as CSV for samples and JSON for events/commands. Exports must retain units or a linked dictionary, quality, and run/boot identity. Full deterministic playback through the UI is an extension.

## Service restart and reconciliation

After a ground-service restart, load retained history as historical state. Do not present a database row from yesterday as a current contact. Start contact status as waiting/stale, reconnect, and request/receive a fresh full snapshot.

Preserve pending command records as unresolved. Do not resend automatically merely because they have no terminal result. A received late result may resolve a record if identities and boot match. If spacecraft boot changed, mark that boundary and retain the uncertainty of old commands.

Reconnection restores an accurate present view from a full snapshot. It need not reconstruct every lost event. Mark gaps in trends; avoid drawing a continuous line that suggests observations were collected through the outage.

## Degraded storage and processing

Select and document a bounded policy for a blocked/unavailable database. A useful baseline is to keep receiving and displaying telemetry through a bounded queue, report persistence degradation, and refuse new state-changing commands when their intent cannot be recorded. Existing commands may still finish; terminal results need a bounded buffer and an explicit data-loss warning if records cannot be preserved.

Do not block the receive loop indefinitely on a write. When a buffer fills, the chosen drop/reject policy must be visible through counters and status. A final-release storage failure scenario verifies this policy and recovery.

Define what “recorded” means for your storage system. Do not claim survival of an abrupt power loss merely because a write API returned unless the chosen durability behavior supports that claim. Normal restart persistence is the baseline; fault-tolerant storage is an extension.

Use limits for history windows, query sizes, browser update rates, and retained diagnostics. The live interface should consume downsampled/limited data as needed rather than requiring every 100 Hz internal sample to cross the browser boundary.

## Scenario controls

Initially use a separate local scenario file, test harness, or developer control surface for fault and link injection. A UI experiment panel is optional. If added, label it clearly and keep its events in the experiment timeline. Ground may observe the configured injection as experiment metadata, but ordinary spacecraft status still comes through telemetry.

## Usability acceptance

A reviewer should be able to answer, using the UI and records:

- What was the last reported spacecraft mode, and is that view current?
- Are these values measured, invalid, synthetic, or old?
- What caused SAFE, and what still prevents NORMAL?
- Was a command sent, accepted, completed, rejected, or left uncertain?
- Did the spacecraft restart, or did only the ground service restart?
- Is history being saved successfully?

Use labels as well as colors for status. Keep numeric units visible. Focus the initial layout on these questions; a realistic-looking console is optional polish.
