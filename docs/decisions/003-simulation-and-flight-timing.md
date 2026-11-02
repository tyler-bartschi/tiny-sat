# 003 — Simulation time, flight scheduling, and critical lag

**Status:** accepted principles, November 1, 2026. Numeric parameters and the open policies below remain undecided.

## Context

The simulation represents spacecraft hardware and its physical environment. Flight software observes that world and requests actions. The developer wants the environment to continue evolving independently when flight disappears, stalls, or restarts.

All components initially run on the Mac. The simulation and flight have independent execution lifetimes; future Raspberry Pi deployment remains possible. This record captures the timing and related interface choices accepted during Phase 0, without selecting implementation code, libraries, or transport.

## Accepted decisions

### 1. Simulation owns physical state and its timeline

Battery energy, temperature, environmental conditions, and applied simulated-hardware state belong to the simulation. Simulation time represents the physical evolution actually calculated by that model.

Physical changes are calculated through explicit simulation steps. The physics step, sensor sampling/publication intervals, and flight activity periods are separate quantities; they need not have identical rates.

### 2. Simulation is free-running

Simulation progression does not wait for flight requests, replies, health checks, or completion of flight activities. Flight disappearance or restart does not by itself stop the environment or reset physical state or simulation time.

The model continues evolving from its applied hardware state. Flight protection cannot execute while flight is absent. Independent hardware-watchdog behavior has not been selected.

Free-running independence does not mean the simulator can guarantee continuous 1x pacing under arbitrary host overload. Critical simulator lag is an experiment failure governed by decision 6.

### 3. The initial interactive simulation attempts 1x pacing

The target is approximately one simulated second per real elapsed second. Real elapsed time is measured directly using a host monotonic clock, relative to a simulation-run reference. It is not derived from sensor timestamps or the most recently observed simulation time.

Simulation time accounts for physical work completed. Comparing it with the run's elapsed-time target exposes unfinished model time. Pacing should account for computation time rather than adding a full step-duration wait after every completed step.

Accelerated execution, coordinated pause, and alternative test-clock modes have not been selected as initial operating modes.

### 4. Flight schedules independently using real elapsed time

Flight periodic scheduling uses its own monotonic elapsed-time measurements. It does not wait for simulation-time messages to release scheduled activities.

Flight uptime can restart at zero while the simulation timeline continues. Observation source timestamps remain simulation timestamps; flight receipt timestamps are local elapsed-time observations. Do not directly subtract timestamps from different clock domains without a defined mapping.

This decision selects the clock for flight scheduling. It does not yet select the clock for every fault-persistence interval, recovery hold, command-expiration deadline, or other behavioral timer.

### 5. Sensor publication and actuator interaction are distinct

Simulation publishes sensor observations at appropriate per-sensor intervals. Flight can consume received observations independently of when they were published; reading an old observation again does not make it a new measurement.

Actuator interaction follows a request-response model: flight requests a supported action and simulation reports its result. Application occurs at simulation-step boundaries as specified in decision 7. Result stages, message encoding, eligibility, and ordering policy remain open.

Neither observation publication nor actuator-response delivery may impose an indefinite wait for flight on the physical model. Buffer limits and communication-failure behavior still need to be designed.

### 6. Critical simulation lag ends the run

Measure simulation lag against the original 1x elapsed-time reference: elapsed real time since that reference minus represented simulation elapsed time over the same interval.

If lag meets the eventual critical condition, end the simulation run and identify it as failed/invalid for its intended 1x timing claim. Do not silently continue in a slower mode or hide the lag by rebasing the pacing reference.

Record the reason and available timing diagnostics. Recovery of pacing before termination does not erase any interaction errors or invalid timing measurements already caused by an earlier delay.

The exact critical threshold, persistence rules, and warning/recovery behavior remain open. The 10 ms step, 50 ms warning, 250 ms critical threshold, and example hold times discussed conversationally were illustrations, not adopted settings.

Whether ending the simulation also shuts down flight and ground is a separate orchestration decision. This policy does not automatically select that behavior.

### 7. Actuator changes take effect at simulation-step boundaries

**Accepted:** November 1, 2026.

Apply eligible, valid actuator requests between physics updates. A request arriving while a physics step is being calculated does not change the hardware state used during that calculation. The simulation's state owner applies changes at an eligible boundary, and subsequent physical evolution uses the resulting state.

This separates request reception from physical application and gives the action a definite simulation-time application point. The communication receiver must coordinate with the simulation state owner rather than mutate applied hardware state partway through a physics update. No queue representation, thread arrangement, or synchronization library is selected by this decision.

The policy does not yet determine which boundary is eligible during catch-up, whether future-scheduled actions are supported, or how multiple requests at one boundary are ordered. Receiving or accepting a request is distinct from applying it; the exact response stages remain open.

## Alternatives and tradeoffs considered

- Lockstep execution can simplify coordinated experiments but would need special handling to keep the environment progressing when flight is unavailable. Free-running execution matches the selected independence goal.
- Scheduling flight from simulation time can support accelerated experiments, but adds dependence on simulation-time delivery. Independent real-elapsed-time scheduling better fits the initial interactive controller model.
- Continuing indefinitely after critical lag preserves an exploratory session but distorts interactions with real-time flight. Ending the run provides an explicit validity boundary for the initial system.
- Bounded catch-up with small physics steps was discussed as a recovery approach. The batch limits, numerical stepping policy, and acceptable lag have not yet been selected.
- Applying actuator changes as soon as possible could support finer event timing but would require coordinating with, or splitting, an in-progress physics interval. Step-boundary application was selected for clear state ownership and event ordering, accepting boundary-related delay.

## Consequences and verification intent

The implementation should demonstrate that flight can stop/restart while physical state and simulation time continue. Measure actual simulation pacing and flight scheduling separately, with their clock domains identified.

Once thresholds are selected, introduce a controlled simulator delay and verify lag reporting and critical-run termination. Test loss of flight separately so it is not confused with simulator overload. Termination and its diagnostics must remain observable without relying on a working flight process.

Restoring pacing cannot reconstruct flight decisions that did not occur during an interruption. Evidence must preserve interruptions and distinguish a valid physical-model computation from a valid real-time closed-loop run.

Exercise actuator requests arriving during a physics update and around its boundaries. Verify that the current update uses its established hardware state, each applied change has a recorded simulation-time boundary, and following updates use the changed state. Catch-up and same-boundary ordering checks require the remaining eligibility policies to be selected first.

## Remaining decisions

- Numerical integration and fixed/variable step policy, plus actual step size.
- Sensor sample/publication rates and flight task periods.
- Warning and critical lag limits, persistence, and warning-clearing behavior.
- Catch-up work bounds and input/output service fairness.
- Actuator eligibility during catch-up and ordering of simultaneous actions; step-boundary application itself is accepted.
- Clock domains for fault qualification/recovery, sensor-age policy, and action validity.
- Observation buffering, reconnect behavior, and any source-to-receiver clock mapping.
- Host sleep/debugger suspension handling and end-of-run orchestration.

## Relationship to the initial specification

The original [spacecraft behavior specification](../02-spacecraft-behavior.md) contains proposals that allowed a flight restart to reset the plant/timeline and scheduled health activities in simulation time. This accepted record supersedes those alternatives for the initial interactive system: the world survives flight restarts, and flight scheduling uses real elapsed time.

The original simulation-time fault-persistence values remain proposals; they are not converted into real-time requirements by this record. The earlier specification and verification targets can be reconciled when the developer explicitly requests those edits.

## Revisit triggers

Revisit if measured host performance cannot sustain the selected model, the project adds fast control dynamics, an accelerated test mode becomes important, or deployment to another computer changes the timing assumptions.
