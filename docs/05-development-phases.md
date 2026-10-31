# 05 — Development phases

## How to progress

Each phase ends with a useful demonstration, a small evidence record, and a decision about readiness. Phases express dependency order, not estimated weeks. Work within a phase in the order that best resolves your uncertainty; this is not a file-by-file implementation recipe.

The core path is **0 → 1 → 2 → 3 → 4 → 5 → 6**. You can explore protocol ideas during Phase 2 or UI sketches during Phase 3, but complete the underlying behavioral gate before depending on it. Extensions follow the core release.

The requirements in [06](06-verification.md) are the final baseline. Their phase column shows when a capability first needs to pass its listed checks; later phases must preserve it. Early local demonstrations do not need capabilities introduced later.

## Phase 0 — Foundation and engineering intent

**Purpose:** make the project small enough to start and the important assumptions explicit.

**Prerequisites:** this specification and your learning goals.

**Scope:** the repository choice, intended component boundaries, local toolchain, model units, time model, state ownership, and first demonstration described in [01](01-phase-0.md).

**Decisions you own:** repository layout, build/test tools, initial model organization, and the level of model abstraction. Identify later decisions without trying to resolve them all.

**Demonstration/evidence:** a short project orientation and initial decision notes that explain how Phase 1 can run independently of the ground system. Include chosen initial conditions and what is deliberately deferred.

**Exit gate:** ORG-01. No application implementation is required to finish this planning gate. When implementation begins, replace proposed run instructions with verified commands as they become available.

**Avoid for now:** thread architecture for every component, deployment automation, custom code generation, and empty framework scaffolding.

## Phase 1 — A local spacecraft that evolves

**Purpose:** learn the plant/observation/flight distinction and produce a small running C++ system.

**Prerequisites:** Phase 0 decisions; a buildable local C++ starting point that you create.

**Scope:** battery energy and temperature, a payload/load input, healthy and invalid sensor observations, BOOT and basic NORMAL behavior, explicit simulation steps, and local state output. SAFE may be a defined placeholder until Phase 2 implements autonomy. Use 10 Hz initially if it reduces friction.

**Decisions you own:** model parameters, numerical integration, ownership interfaces, initial configuration format, and local reporting format.

**Demonstration:** run a short nominal case, vary sunlight or load, and explain why the trajectories differ. Run the same deterministic case twice and compare states and event order. Make one sensor invalid and show that invalid is distinct from zero.

**Exit gate:** SIM-01, SIM-02, FC-01; V01, V02, and V03 at Phase 1 scope. Show units, bounds, valid startup, time behavior, and reproducibility. Record any known gap between a local placeholder and the later full mode contract.

**Evidence:** initial model plots or tabular samples, parameter rationale, and the exact scenario/configuration used.

**Avoid for now:** networking, browser work, orientation dynamics, and a highly realistic orbital environment.

## Phase 2 — Autonomous flight behavior and measured scheduling

**Purpose:** make the flight computer do meaningful work without ground support.

**Prerequisites:** a repeatable model and valid observations from Phase 1.

**Scope:** the mature multi-rate profile or your recorded alternative; task timing instrumentation; BOOT/NORMAL/SAFE transitions; low-battery, overtemperature, and required-observation timeout protection; recovery guards; local command entry for payload, modes, and sensor reset. Local entry may be a test harness or console; no network protocol is required yet.

**Decisions you own:** executive/concurrency model, release ordering, overrun policy, fault persistence, hysteresis values, mode arbitration, and the meaning of reset completion.

**Demonstration:** enable the payload, induce overheating, observe SAFE disabling the load, remove the disturbance, and show that recovery qualification does not itself exit SAFE. Request NORMAL too early and receive a reasoned rejection; request it after recovery and succeed. Repeat with low battery and a required-sensor timeout. Include overlapping faults.

**Exit gate:** SIM-03 and FC-02 through FC-06; V04 through V07, plus the mature V03 lifecycle. Measure timing using B01. Use deterministic time for fault boundary checks and a real-time run for scheduler evidence.

**Evidence:** timing summary, transition timeline, chosen thresholds, and one example of a deliberately rejected local action.

**Avoid for now:** building an RTOS-like framework or claiming hard real-time behavior from desktop measurements.

## Phase 3 — A real protocol and a minimal ground client

**Purpose:** prove end-to-end exchange before building a dashboard.

**Prerequisites:** stable flight behavior; a decision on transport and wire encoding.

**Scope:** separate flight and ground processes; binary encoding/decoding; integrity and bounds checks; a full telemetry dictionary; command/result correlation and validation; a minimal CLI or textual ground view. Document boot/session IDs, sequencing, and the intended expiry/retry design. Default to no automatic retries until Phase 5 verifies duplication safety.

**Decisions you own:** exact packet layout and integrity variant, ground-service language, endpoint configuration, message dictionary, framing, initial error counters, and result representation.

**Demonstration:** start flight without ground, then connect ground and receive current status. Send a payload command and observe its result and resulting telemetry. Send a malformed packet and a valid-but-disallowed command; distinguish decode rejection from domain rejection. Stop ground and show continued autonomous flight.

**Exit gate:** LINK-01, PKT-01, PKT-02, CMD-01, CMD-02, TM-01, OPS-01; V08, V09, V10, and V16 at clean-link scope. Both endpoints agree on independent golden fixtures. Document the prototype's unverified retry/restart limits.

**Evidence:** protocol contract, fixture descriptions, example command/result exchange, and a clean-link integration summary.

**Avoid for now:** mixing binary-protocol debugging with chart rendering or relying on a browser to validate flight behavior.

## Phase 4 — Operator interface and durable history

**Purpose:** turn the textual client into useful ground control.

**Prerequisites:** a working clean-link ground service and message dictionary.

**Scope:** ground API, browser status/trends/commands/events, local contact freshness, persistence, restartable history, bounded queries, and export. Basic stale handling is required now; Phase 5 adds rigorous out-of-order/session reconciliation.

**Decisions you own:** service/UI contract, database and schema, retention limits, UI layout, chart gaps, backend heartbeat, and storage durability claim.

**Demonstration:** operate the spacecraft from the browser, view a fault timeline, disconnect the browser/backend path, and show that the display stops claiming live state. Restart ground and show preserved history while waiting for fresh contact. Export a short segment with units, quality, and identities.

**Exit gate:** GND-01 through GND-03; V13 and V14 at this phase's scope. The UI communicates pending and terminal command states without optimistic spacecraft-state changes.

**Evidence:** screenshots, a small export, a restart/history check, and the recorded API/storage decisions.

**Avoid for now:** elaborate 3D spacecraft graphics, many dashboard pages, and full mission replay.

## Phase 5 — Unreliable links, uncertainty, and reconciliation

**Purpose:** make distributed behavior correct when the happy path breaks.

**Prerequisites:** clean-link behavior and an inspectable command/history view.

**Scope:** directional impairment; targeted and probabilistic packet faults; bounded retries; duplicate suppression; identity conflicts; enforced command expiration; late results; outcome-unknown handling; sequence/wrap behavior; full-snapshot reconciliation; ground/flight restart boundaries. Complete the protocol revision if Phase 3's prototype needs one.

**Decisions you own:** replay rejection, deduplication/result retention, retry spacing and budgets, command validity clock, session transitions, impairment queue/order policy, and historical handling of late packets.

**Demonstration:** drop a reset-command result, retransmit the same request, and prove the reset count changes once. Delay a state-changing command past validity and prove no effect occurs. Interrupt communications for 30 seconds while a fault qualifies, reconnect, and recover the present view. Restart flight while old packets remain delayed and prove they cannot control the new boot.

**Exit gate:** PKT-03, CMD-03 through CMD-06, TM-02, GND-04, IMP-01, IMP-02; V11, V12, V15 through V18. Re-run affected clean-link checks. Use B03 through B06 and targeted injections, not random loss alone.

**Evidence:** command timelines, reset-counter evidence, sequence/session checks, impairment config/seed, ordered injection records, and explicit limits of historical reconstruction.

**Avoid for now:** promising exactly-once execution across power loss or implementing a general reliable transport stack.

## Phase 6 — Verified and reviewable first release

**Purpose:** turn the working system into a reproducible portfolio artifact.

**Prerequisites:** all functional phases, a fixed release-candidate configuration, and no unresolved contradiction in the specification.

**Scope:** resource bounds and overload checks, unavailable storage, invalid startup configuration, graceful/abrupt shutdown, consolidated evidence, a final integrated scenario, and verified setup/run instructions. Add appropriate local/CI checks once they are useful and repeatable.

**Decisions you own:** release packaging, exact supported environment, final target values, evidence presentation, and a candid limitation list.

**Demonstration:** the mission story in V22, along with a 60-minute stability run under B07 and the focused overload/storage scenarios. Have a clean checkout or equivalent clean environment follow the documented build/run path.

**Exit gate:** GND-05, OPS-02, OPS-03, VER-01, VER-02, REL-01, and the full requirements register. V19 through V23 complete the new checks; repeat earlier scenarios where the release changed their assumptions or implementation. Present measurements against the selected targets and keep failures visible until resolved or scope is explicitly revised.

**Evidence:** release summary, requirements-to-results mapping, scenario/configuration versions, environment, timing/resource plots, screenshots/video, and known limitations. A short architecture explanation should describe your implemented design and its tradeoffs, replacing any early assumptions that no longer apply.

**Avoid for now:** adding a new subsystem during release verification. Put it in the extension backlog and finish the core.

## Suggested stopping points

After Phase 2 you have an autonomous flight-computer simulation. After Phase 4 you have an integrated clean-link spacecraft/ground system. After Phase 6 you have the intended resilient portfolio project. These are useful intermediate accomplishments, but they are different completion claims.

If a phase feels too large, split it into a local demonstration and an integration demonstration without weakening its exit gate. If a numeric target is inappropriate, revise it with a reason and preserve the old measurement. If a requirement itself changes, update the relevant behavior, phase, scenario, and decision records together.
