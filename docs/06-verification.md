# 06 — Requirements, benchmarks, and verification

## Verification policy

This is the authoritative baseline requirements register. Each requirement has an introduction phase and verification scenarios. The scenario descriptions are intended test designs, not test implementations or claims of passing results.

All requirements are proposed first-release scope. A behavior marked **must** is required once you adopt this baseline. Numeric profiles below are **suggested targets**: adopt them or replace them with explicit values and rationale before acceptance. A target with no selected value cannot be reported as passed. Changes to requirements need corresponding updates to behavior, phases, and verification mapping.

Use four complementary forms of evidence:

- **Inspection:** contracts, decision records, configuration, and source/design review.
- **Focused checks:** deterministic model, state-machine, parser, and clock-boundary checks.
- **Integration scenarios:** separate actual flight/ground processes and selected storage/UI behavior.
- **Measurement/demonstration:** timing/resource data and an operator-visible mission story.

Keep at least one independent oracle for important behavior. Examples include an analytical constant-power expectation, a manually specified golden packet, a reset counter, or an expected state-transition predicate. Two endpoints agreeing on the same encoding bug is not sufficient evidence.

## Baseline requirements register

| ID | Required observable outcome | Phase | Verification |
| --- | --- | --- | --- |
| ORG-01 | Repository strategy, initial ownership/time decisions, selected first increment, and assumptions are recorded | 0 | V00 |
| SIM-01 | Plant truth and timestamped/quality-marked sensor observations are distinct; invalid observations are not numeric zero | 1 | V01, V02 |
| SIM-02 | Units, bounds, time domains, initial conditions, deterministic input ordering, and model reproducibility are documented and checked | 1 | V01, V02, V21 |
| SIM-03 | Payload and sunlight causally affect power/thermal evolution; SAFE removes payload load; a defined scenario permits recovery | 2 | V01, V05, V06 |
| FC-01 | Flight starts through BOOT with payload off and reports identity and state; startup cannot enter NORMAL with unsafe/unavailable required observations | 1, complete in 2 | V03 |
| FC-02 | Multi-rate activities run under an explicit overrun policy, with execution, lateness, deadline, and skipped-work measurements | 2 | V04, V19 |
| FC-03 | Invalid/missing required observations expose quality/age and trigger timeout protection at the selected threshold | 2 | V02, V07 |
| FC-04 | Sustained low power, overtemperature, and required-observation timeout cause SAFE and payload disable within the selected protection bound | 2 | V05, V06, V07 |
| FC-05 | Fault recovery uses declared guards; SAFE remains latched until a valid NORMAL request; unresolved concurrent faults block recovery | 2 | V05, V06, V07 |
| FC-06 | One logical flight authority arbitrates state; protection takes precedence over conflicting requests | 2 | V07, V10, V19 |
| LINK-01 | Separate spacecraft and ground processes exchange commands and telemetry without shared runtime state | 3 | V08 |
| PKT-01 | A versioned binary contract defines encoding, bounds, identity, sequence, time, integrity variant, and golden fixtures | 3 | V08, V09 |
| PKT-02 | Malformed, unsupported, truncated, oversized, or integrity-failed input has no command effect and does not crash the receiver | 3 | V09, V19 |
| PKT-03 | Version rejection, sequence ordering/wrap, and retirement of old-boot data follow the documented policy | 5 | V09, V15, V17 |
| CMD-01 | Supported commands have correlation and defined accepted/rejected/completed/failed meanings; sent does not imply completed | 3 | V10 |
| CMD-02 | Flight validates parameters, current mode, reset-busy state, and protective preconditions at the point of effect | 3 | V10, V07 |
| CMD-03 | A duplicate command cannot repeat its non-idempotent effect within the supported boot/validity scope; identity/payload conflicts are rejected | 5 | V11, V18 |
| CMD-04 | State-changing commands target a boot and enforce a bounded validity condition on spacecraft admission and before queued execution | 5 | V12, V17 |
| CMD-05 | Missing results yield outcome unknown; late results can reconcile matching records without rewriting history | 5 | V11, V12, V16 |
| CMD-06 | Restart boundaries cannot silently retarget old commands, reuse conflicting identities, or claim durable exactly-once execution | 5 | V17 |
| TM-01 | Full status snapshots carry mode, faults, payload, power/thermal observations, quality/age, identity, timing, and diagnostic information as defined | 3 | V08, V10 |
| TM-02 | Old/duplicate telemetry does not roll back current state or refresh contact; boot/field/time distinctions remain visible | 5 | V13, V15, V17 |
| GND-01 | UI presents status, units, quality, trends, freshness, events, and command lifecycle; backend disconnect is visible | 4 | V13 |
| GND-02 | Ground submits requests through the service without directly altering flight truth; stale/disconnected state blocks new state-changing submissions | 4 | V10, V13, V16 |
| GND-03 | Telemetry, command attempts/results, and events survive a normal service restart and support bounded history/export | 4 | V14, V17 |
| GND-04 | Reconnection obtains accurate present state; gaps and unresolved commands remain explicit across service/spacecraft restarts | 5 | V15, V16, V17 |
| GND-05 | Storage failure and full ground buffers have visible, bounded behavior without indefinitely blocking reception | 6 | V20 |
| IMP-01 | Independent uplink/downlink impairments support delay, jitter, loss, corruption, duplication, reordering, and outages with recorded configuration | 5 | V11, V12, V15, V16, V18 |
| IMP-02 | Command retries, accepted work, impairment buffers, and result/dedup retention have finite declared limits and safe overflow policies | 5 | V11, V18, V19 |
| OPS-01 | Loss/absence of ground does not stop spacecraft sensing, health logic, or autonomous protection | 3 | V08, V16 |
| OPS-02 | Overload and shutdown follow bounded policies, expose dropped/skipped work, and preserve protective processing | 6 | V19, V20 |
| OPS-03 | Invalid configuration is refused; subsystem reset, process restart, and scenario reset have documented distinct effects | 6 | V03, V10, V17, V23 |
| VER-01 | Acceptance evidence records versions, scenario/configuration, seeds, environment, selected targets, results, and limitations | 6 | V21, V22, V23 |
| VER-02 | Focused and integrated checks cover nominal behavior, fault boundaries, parser errors, uncertainty, restart, and overload with independent expectations | 6 | V01–V22 |
| REL-01 | A clean setup/run path and mission demonstration reproduce the claimed capabilities without undocumented manual database edits | 6 | V22, V23 |

The listed checks are cumulative. For example, V19 hardens a parser introduced in Phase 3; it does not postpone all parser verification until Phase 6. V21 consolidates reproducibility evidence that begins with the Phase 1 model checks.

## Suggested benchmark profile

Record adopted values in your scenario/configuration and evidence summary. These defaults are intended to be achievable and informative, not universal spacecraft requirements.

### B01 — Nominal scheduling

- Mature task profile: plant/sensor work at 100 Hz, health evaluation at 10 Hz, periodic telemetry at 1 Hz, all in simulation time.
- Run the 1× real-time runner for 30 elapsed minutes after a declared warm-up on the recorded Mac environment.
- Suggested nominal target: 99th-percentile start lateness no greater than 5 ms for the 100 Hz activity, with no more than 1% missed completion deadlines. Use the next scheduled release as the default completion deadline unless you select another explicit deadline.
- Count skipped releases as missed work, not as timely executions. Record total scheduled releases, starts, completions, skipped releases, and unfinished work at the measurement boundary.
- Report per-task lateness percentiles/max, execution-duration percentiles/max, misses, skipped work, and simulation-time/elapsed-time ratio. Account for warm-up and boundary effects explicitly.
- Suggested pacing target: simulation-time/elapsed-time ratio from 0.99 to 1.01 over the nominal measurement window.
- If the host or chosen workload cannot meet the target, report the measurement, investigate, then adopt a justified target change before the next acceptance run. Do not claim the original target passed.

For this real-time benchmark, map planned simulation releases onto host monotonic time from a fixed run anchor and the selected 1× pace. Compare actual starts and deadlines in that host clock domain. Do not silently move the anchor after a stall: slowing the simulation must remain visible as pacing drift and late/unserved releases, even if each model step itself was cheap. Deterministic simulation checks can use simulation-time release counts separately.

An average of 100 executions per second does not prove even spacing. A model that runs 100 simulated releases quickly also does not prove it maintained 100 releases per elapsed second.

### B02 — Clean-link responsiveness

At 1× pacing on loopback with no impairment, send at least 20 individually tracked immediate commands, including valid no-ops and rejected requests. Suggested target: each receives a terminal response within 1 elapsed second of initial send. Measure sensor-reset procedure completion separately against its configured duration plus communication/processing allowance.

Suggested UI target: a newly received snapshot or result becomes visible within 2 elapsed seconds of service receipt. Keep this distinct from source-to-ground latency.

### B03 — Routine impairment

Run a 10-elapsed-minute integrated scenario at 1× with the following proposed settings in each direction: 250 ms fixed delay, additional independent uniform jitter from 0 to 100 ms, 5% independent message loss, 1% selected payload corruption, and 2% duplication. Record operation order, independent seeds/random streams, and queue limits. Exercise reordering deliberately in V15 as well.

No crash, silent repeated effect, state rollback, or unbounded backlog is acceptable. Some commands may remain outcome unknown; delivery of every command is not an acceptance condition under loss. Targeted checks must prove duplicate and lost-result cases independently of the random run.

### B04 — Outage and reconnection

Use a 30-simulation-second bidirectional outage at 1×. For this default profile, drop messages during outage and discard the link's pre-outage pending delivery queue on entry; separate targeted tests retain delayed traffic to exercise expiry/restarts.

At the suggested ground thresholds, stale appears after 3 elapsed seconds without advancing telemetry and offline after 10. After the outage ends, require a post-restoration full snapshot and a current UI view within a suggested 5 elapsed seconds using only B03's bounded delay/jitter. Disable loss, corruption, duplication, and forced reordering during this restoration timing check. Do not promise this bound during unlimited random losses or corruption.

### B05 — Command bounds

Suggested values: at most 16 pending ground commands, 3 total transmission attempts per command, at least 2 elapsed seconds between attempts, and a 10-simulation-second validity lifetime for state-changing commands. Retry spacing, lifetime, and maximum future deadline horizon must be mutually consistent with selected link delays.

Declare the flight accepted-work limit, dedup/result retention capacity, and safe eviction/replay policy before testing. Fill and exceed each relevant limit in V18. The fixed capacities are your decision; the requirement is bounded behavior without duplicate effects.

### B06 — Fault timing and recovery

Use the threshold/persistence/recovery profile in [02](02-spacecraft-behavior.md), or record replacements. Suggested bound: action by the next health evaluation after a fault qualifies, no more than 0.1 simulated seconds at 10 Hz. A physical threshold crossing can incur an additional sensor-sampling interval before the persistence timer starts.

Measure from the right event: injected disturbance, first hazardous valid observation, qualification, SAFE commit, and ground observation are different timestamps. During a nominal 1× run also report wall time and any host overrun. During overload select and verify a finite wall-time protection budget; 1 elapsed second from qualification is a suggested starting target.

### B07 — Release stability

Run for 60 elapsed minutes at 1×: 30 nominal minutes followed by 30 minutes using a recorded mixture of impairments, bounded command traffic, and fault/recovery cycles. This can include the B01 nominal timing window and B03 impairment window; do not repeat equivalent long runs without a reason.

Record peak and time-series process memory, queue high-water marks, retained-record counts, drops, task timing, and final state. Set queue and in-memory retention capacities before the run and verify they are never exceeded. Choose a machine-specific memory budget before acceptance. Disk history may grow according to a documented retention policy; that is different from an unbounded in-memory queue.

Acceptance requires no crashes/deadlocks, no unexplained retained-state growth beyond the declared bounds, explicit unknown outcomes where appropriate, and continued protection. A 60-minute run supports only a 60-minute stability claim under its stated conditions.

### B08 — Overload and storage failure

For 60 elapsed seconds, offer at least 10× the configured accepted command/ingress budget, including malformed input, and keep the source rate bounded. Trigger a protective condition during load. Verify finite queues, visible rejection/drop counters, bounded protective response, and return to normal processing within a suggested 5 elapsed seconds after excess load stops.

Separately make storage unavailable for 30 elapsed seconds and fill the selected buffer. Verify the documented refusal/retention/drop policy and recovery. The baseline need not recover data already explicitly reported lost.

## Verification scenarios

Each scenario result should name its selected profile and record actual observations. Boundary cases can be small deterministic checks; they do not all need a full UI session.

### V00 — Foundation inspection

Review the Phase 0 decision set, repository plan, model units, initial conditions, and first increment. Pass when the early decisions have reasons, later decisions have a phase, and plant/flight/ground ownership is understandable. This is a document review, not a runtime test.

### V01 — Model causality, units, and bounds

Use a constant net-power case to compare battery energy against an independently calculated expectation. Exercise zero/full battery boundaries and invalid capacity. Compare payload-on/off thermal and power trajectories from identical initial conditions. Check finite values and integration-step sensitivity. In Phase 2 include a SAFE recovery scenario whose available charging and cooling actually permit recovery.

### V02 — Observation quality and clock behavior

Exercise healthy, invalid, non-finite, and missing observations; confirm last-good value, quality, source time, and age remain distinct. Check that flight cannot substitute plant truth through the ordinary observation path. Repeat deterministic inputs. Alter wall-clock presentation or use a controllable clock boundary and verify simulation intervals/monotonic timers remain correct. Test zero/negative elapsed inputs according to your model contract.

### V03 — Startup and lifecycle

Phase 1 checks identity, payload-off startup, explicit initialization, and valid nominal entry. Phase 2 adds unsafe initial battery/temperature, missing observations through the BOOT deadline, startup failure, and SAFE entry. Phase 6 includes invalid rates/thresholds/endpoints and verifies refusal before an apparently normal run starts. A new boot has a new identity.

### V04 — Multi-rate scheduling and overrun

Use deterministic releases to check activity counts and ordering, including a deliberately slow activity and the selected missed-release policy. Then measure B01 in real time. Verify skipped work and late completion cannot disappear from the denominator. Use measured durations rather than log-line counts alone.

### V05 — Low-power protection

Start near the low threshold with a controlled energy trajectory. Check below/equal/above entry threshold, disturbance shorter than persistence, exact qualification, and protective timing. Try NORMAL before recovery. Restore charging, check the recovery threshold and hold interval, and require an explicit successful NORMAL request. Preserve the payload-off effect and event sequence as evidence.

### V06 — Thermal protection

Repeat the threshold, persistence, and recovery checks for temperature. Confirm that SAFE changes load rather than teleporting temperature. Exercise a scenario where SAFE cannot immediately cool because the environment remains hot; the software must remain protected and explain why recovery is blocked.

### V07 — Sensor timeout and state arbitration

Stop required observations, inject explicit invalid samples, reset a sensor, and check the timeout/recovery intervals. Overlap a thermal and low-power or sensor fault. Attempt payload enable and NORMAL requests while protection qualifies. Confirm protection wins, one cleared fault does not clear the others, and active/cleared state is explainable. Include a request whose preconditions change while it waits for execution.

### V08 — Clean protocol integration

Start actual separate flight and ground processes. Decode full telemetry, send each supported command, inspect results, and compare UI/CLI values with the wire dictionary. Start/stop ground without stopping flight. Check that no shared-memory shortcut supplies ground's operational state. Golden fixtures are checked independently of live round trips.

### V09 — Parser, integrity, and version cases

Include empty input, truncated headers/payloads, illegal length, oversized input, trailing bytes, unsupported versions/types, invalid enum/range, non-finite numeric payloads where applicable, and selected bit-corrupted frames. Check the integrity algorithm against its published/reference vector and your golden packets. No malformed input may mutate commanded state. Valid input afterward must still work. Phase 5 adds the declared sequence-wrap boundary cases; Phase 6 includes bounded randomized malformed input and available runtime diagnostics/sanitizers appropriate to your C++ toolchain.

### V10 — Command meanings and validation

Exercise all command intents, valid no-ops, domain rejections, unsupported sensor IDs, reset-in-progress handling, and acceptance versus completion. Verify the effect and the reported result agree. Use B02 for immediate-command latency. Confirm the UI cannot bypass server/flight validation and never presents a click as confirmed spacecraft state. In Phase 5 deliver accepted and terminal responses out of order: an old acceptance must not replace completion, and a contradictory terminal report must surface a consistency error.

### V11 — Lost result, duplicate, and identity conflict

Target one sensor reset. Drop its initial result, retry the same immutable command, and prove exactly one reset-count increment in that boot. Exercise duplication while accepted work is pending and after completion. Send the same identity with a changed payload and reject it. Then drop all results: ground must report outcome unknown even if the effect occurred. Deliver a late matching result and preserve the history of uncertainty as it resolves.

### V12 — Delayed and expired command

Delay a state-changing request past its spacecraft-enforced deadline and verify no effect. Repeat with admission before expiry but queued execution after expiry. Check malformed/far-future deadlines under the selected horizon policy. Verify retries do not refresh the deadline and ground's local timeout cannot be mistaken for spacecraft rejection. An expired duplicate whose original history is unavailable must not become evidence that the original effect never occurred.

### V13 — UI freshness, quality, and backend disconnect

Observe nominal updates, bad field quality in a new snapshot, a stale sensor in fresh telemetry, and the stale/offline thresholds after telemetry stops. Disconnect the browser/service path separately. Check labels, units, command gating, source versus receipt times, and gaps in charts. Phase 5 repeats with old/duplicate packets that must not refresh the live indicator.

### V14 — Persistence and export

Record a short telemetry/fault/command timeline, restart the service normally, and query/export it. Check values, units, quality, source/receipt times, identities, attempts, and terminal results. Verify bounded history queries and the policy for duplicate/late records. Establish the actual normal-restart durability boundary without claiming untested crash durability.

### V15 — Reordering and current-view reconciliation

Deliver newer telemetry before older telemetry, interleave duplicates, and inject a selected reordering event in each direction. Current state must not move backward or falsely renew freshness. Retain late samples historically if that is the selected policy. After missing several full snapshots, a new full snapshot restores the present view without requiring the lost packets.

### V16 — Outage and autonomous operation

Run B04 while flight health tasks continue. Trigger a fault during the outage. Ground ages its view and blocks new state-changing submissions; flight enters SAFE independently. Reconnect, obtain fresh present state, keep history gaps, and leave unresolved commands uncertain unless actual correlated evidence resolves them. Run an uplink-only and downlink-only interruption as separate cases so the UI does not equate one direction with the other.

### V17 — Ground and flight restart boundaries

Restart ground with pending records: preserve history, start a new origin session or verify restored identity state, and do not blindly resend. Restart flight with traffic delayed in the link: require BOOT/new identity, reject old-target commands, reject/quarantine old-boot telemetry/results, and preserve uncertainty of pre-crash effects. State whether plant state survived; a full scenario reset must be visibly different. Test command/result identity collisions across these boundaries and delayed prior-run traffic after an explicit new experiment starts.

### V18 — Capacity, retention, and replay

Fill ground pending commands, flight work admission, impairment delivery queues, and dedup/result capacity. Exceed each cap and verify the declared outcome. Force old-result eviction, then redeliver the old request: it must not regain permission to repeat an effect. Use an explicit floor/expiry/retention policy oracle. Check B05's total-attempt limit and pacing, including a period with no results. Verify that retiring active pending records preserves history without restarting work, and that the declared boot/session metadata capacity has a safe exhaustion policy.

### V19 — Overload, blocking work, and shutdown

Use B08 and force slow/busy non-protective work. Observe scheduler health, memory/queue limits, drop counters, protective response, and recovery when load stops. Shut down with pending work and verify no indefinite join/wait, a documented unresolved-command outcome, and ground freshness timeout. Abrupt termination is permitted to truncate final local records; it must not produce invented completion evidence after restart.

### V20 — Unavailable or slow storage

Block writes or use an intentionally unavailable storage target. Exercise the chosen buffer-full policy, UI persistence warning, new-command refusal, handling of already-running commands, receive-loop responsiveness, and recovery when writes resume. Check for explicit loss accounting rather than silent gaps. Use a bounded history query under live traffic to exercise processing limits as well.

### V21 — Reproducibility and evidence consolidation

Repeat the deterministic model scenario with the same version/configuration/seeds and ordered inputs. Compare discrete decisions exactly and floating-point state using declared absolute/relative tolerances selected for the model. Exclude incidental UTC timestamps and runtime durations from semantic equality. For a live-process impairment failure, retain the ordered trace in addition to seeds; do not promise bit-for-bit live execution from a seed alone.

### V22 — Integrated mission and stability

Run B07 and capture an 8–12 minute demonstration segment or equivalent short scenario: boot, normal telemetry, payload action, targeted lost reset result/duplicate-safe retry, outage with autonomous protection, restored contact, premature NORMAL rejection while current telemetry is available, qualified recovery, and explicit NORMAL acceptance. Scenario-controlled disturbances and preselected initial conditions should make the story practical without disguising their origin. Correlate UI, events, command identities, and measurements. Demonstrate uncertainty where evidence is intentionally absent.

### V23 — Clean setup and release review

Follow documented prerequisites, build, launch, stop, configuration, reset, and demo instructions from a clean checkout/environment. Verify actual commands and expected outputs. Inspect the completed requirement-to-evidence table, supported environment, known limitations, and selected target changes. The next reviewer must not need undocumented state or manual database surgery.

## Result record

For each phase gate or final scenario, retain a concise record containing:

- Scenario/check ID, purpose, and linked requirements.
- Source revision, protocol/configuration/scenario versions, seeds, and ordered inputs where needed.
- Environment: OS, hardware, toolchain, execution mode, selected rates/thresholds, warm-up, and duration.
- Expected outcome and adopted numeric targets, recorded before the run.
- Actual outcome, measurements/counters, representative artifact references, and pass/fail conclusion.
- Known limitations, failures, and follow-up decision or issue.

Add a result column or separate linked evidence index when implementation begins. Until then all runtime checks are **not run**. Keep small summaries in version control; store large recordings separately with provenance and a usable reference.

## Final release gate

Every baseline requirement must map to completed evidence, or be explicitly removed through a scope revision that updates this specification. No critical behavior may depend on an unresolved placeholder or an unrecorded target. The demo and timing claims must match the implemented system and tested environment.

Coverage percentage, the absence of crashes in one run, and a screenshot of a healthy dashboard are supporting facts, not substitutes for the behavioral scenarios above.
