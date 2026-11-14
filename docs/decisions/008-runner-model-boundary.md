# 008 — Runner/model boundary and explicit power-model inputs

**Status:** accepted, November 2, 2026. The supplied interval uses `std::chrono::duration<double>` as specified below. Other concrete interfaces and local numerical/error-handling details remain the developer's choices.

## Context

The initial power model is defined in [decision 007](007-initial-power-model.md). The simulator's operational event loop is defined in [decision 004](004-simulation-flight-tcp.md), with time ownership and pacing in [decision 003](003-simulation-and-flight-timing.md).

The developer wants to begin implementing the power model independently of the surrounding real-time runner, networking, and sensors. Separate calculation of physical evolution from orchestration, and pass the changing physical inputs and simulation interval explicitly.

## Accepted decisions

### 1. The runner orchestrates simulation execution

The runner coordinates simulation progression, real-time pacing, due model updates, sensor sampling, eligible actuator changes, communication, and logging handoffs. It can delegate these responsibilities to components; this is not a requirement for one large loop body.

The runner holds the authoritative current physical state and adopts the next state returned by the model after a successful update. A later update starts from that adopted state. The simulator as a whole remains the owner of world state and simulation time.

### 2. The power model receives starting state and interval

Each calculation receives the starting physical state, the simulation interval to represent, and the changing physical inputs for that calculation. It returns a new physical state and an interval report. It does not advance a hidden battery state independently or modify the caller's starting state as the means of reporting the result.

The model calculates physical evolution rather than measuring how long its own calculation took. It does not read the host clock to derive the interval, sleep to maintain 1x pacing, open sockets, or perform file output. Timing measurements and pacing belong to the runner. Information useful for logging can be returned for the runner to handle.

Supply the elapsed simulation duration using the type selected in decision 7 below. If absolute simulation timestamps are also needed, they are explicit inputs or runner-assigned metadata rather than readings from a hidden real-time clock. Preserve the units selected in decision 007: seconds, watts, and watt-hours.

### 3. Initialize the model with constant configuration

The model may retain configuration that is constant for the configured experiment, including battery energy capacity and baseline/essential power draw. This configuration is distinct from the evolving physical state supplied to each update.

Do not put the current stored battery energy in both model-owned hidden state and runner-owned state. State of charge remains derived from energy and capacity.

Exact constructor/configuration types, names, storage layout, and any unit wrappers are not prescribed. Changing configuration during a run is outside this initial contract unless later explicitly designed.

### 4. The runner supplies changing conditions

Supply current available generation and additional payload draw to the model as changing inputs. Total requested load includes the configured essential draw plus the supplied additional draw; do not count the same payload contribution twice.

For the initial switchable constant payload in decision 007, the supplied additional draw is zero when disabled and the configured payload load when enabled. The runner and actuator/scenario logic determine these conditions. The physical calculation does not decode TCP requests or decide whether flight is authorized to request a change.

The rule for inputs changing within an interval, including scenario changes and catch-up eligibility, still needs to be selected. External actuator application remains subject to the accepted step-boundary policy.

### 5. Return physical state and interval accounting

The new state describes physical conditions at the interval's end. The report describes the physical accounting over that interval, including the supplied, unmet, and curtailed energy needed to verify decision 007.

The report is not automatically a sensor observation or a wire message. Exact fields and representation remain open. The runner can associate it with the interval and use its information for diagnostics and later model coupling. Cumulative accounting, if needed, requires an explicit aggregation responsibility rather than confusing one interval's values with run totals.

### 6. Keep sensing separate from physical evolution

Sensor logic samples physical state according to its own configured schedule and produces defined observations for flight. An observation has measurement time and quality semantics separate from the physical calculation report. A physical update need not produce a new sample from every sensor.

Initially a healthy sensor may report a physical value exactly. Later missing, invalid, or noisy observations can be introduced without changing the underlying battery evolution. An observation failure does not itself change physical stored energy.

The sensor boundary is a logical responsibility inside simulation, not a requirement for another process or thread. Detailed sensor APIs and fault behavior remain separate implementation work.

### 7. Represent the supplied interval as a fractional-second duration

Use `std::chrono::duration<double>` for the interval supplied to the physical model. Its default tick period is one second and its floating-point representation preserves fractional seconds, subject to numerical precision. An interval expressed as 10 milliseconds therefore converts to approximately 0.010 seconds rather than being truncated to zero whole seconds.

The runner supplies this duration; creating or passing it does not read a clock or cause a wait. Interpret its numeric count as seconds for the power model's existing W/Wh formulas. When starting with another chrono duration unit, convert to the selected representation before treating a count as seconds.

A descriptive alias such as `SimulationInterval` may name this type. The alias's namespace and header location remain to be selected. An alias does not introduce a distinct clock domain or prevent real-time durations from being passed accidentally. Input validation remains necessary: the duration type alone does not enforce a positive finite interval or decide zero-duration behavior.

## Alternatives and consequences

A model that reads a clock itself could be convenient for a standalone prototype, but would couple numerical expectations to execution timing or require a controllable clock dependency. Explicit intervals and inputs allow deterministic model checks without starting the real-time runner.

A model could instead own mutable physical state and update it in place. The developer selected explicit starting-state input and new-state output so the transition is visible to the caller. The implementation may still be an object retaining constant configuration.

Given the same valid configuration, starting state, conditions, and interval, the model should produce repeatable results within the selected numerical conventions. No particular class hierarchy, generic subsystem framework, separate library target, or allocation/copying strategy is selected.

## Verification intent and first implementation scope

Exercise the model directly from Catch2 using explicit starting states, conditions, and intervals. No socket connection, operating-system sleep, sensor implementation, logging worker, or complete runner is needed to verify the initial physical calculations.

Use the independent energy-balance and boundary checks in decision 007. Verify that the supplied starting state remains unchanged and the returned state/report describe the requested interval. Chain updates by using a previous returned state as the next starting state. Compare equivalent constant-input durations within declared tolerances, including full/empty boundaries.

A model-only check may represent a long simulation interval without waiting that duration in real time. That tests physical accounting; it is not evidence that the interactive simulator maintains 1x pacing.

Check conversion from milliseconds to the selected fractional-second duration and verify that short intervals remain nonzero. Use the resulting seconds consistently with the Wh energy-accounting formulas.

The developer will implement the model. This record adds no source code, build configuration, or tests.

## Local details to settle during initial implementation

- Concrete configuration, physical-state, changing-input, and result/report representations; the interval type is accepted. Its alias name, namespace, and header placement remain open.
- Numerical type, tolerances, and handling of invalid or non-finite values.
- Whether a zero-length interval is a valid no-op or an error; negative intervals must not represent ordinary forward evolution.
- The assumption about input constancy during an update and treatment of input-change boundaries.
- Small reference values and independently calculated expectations for first tests.

These are local contract choices. The broader TCP wire protocol, operational sensor implementation, thermal equations, and complete real-time event loop are not prerequisites to implementing this power-model calculation.

## Relationship to other records

This record resolves the runner/model ownership and interface direction left open in decision 007. Physical state is caller-owned, model configuration may be retained, and results are returned explicitly. The accepted free-running environment and single operational event loop remain unchanged.

Other files remain unchanged. Reconciliation edits require the developer's explicit request.

## Revisit triggers

Revisit if profiling identifies a concrete cost in state transfer, model coupling requires a different update contract, or additional physics introduces event/integration needs that explicit interval updates cannot adequately express.
