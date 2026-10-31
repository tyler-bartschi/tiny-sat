# 02 — Spacecraft and flight-computer behavior

## The three kinds of state

Distinguish **plant truth**, **sensor observations**, and **flight decisions**. Plant truth is the simulator's internal battery energy or temperature. A sensor observation is what a subsystem reports, with a time and quality. Flight decisions are modes, commands, and protective actions derived from those observations.

The ground station has a fourth view: the last state reported through the link. It can be older than all three. Keeping these distinctions visible is central to the project.

In the first model a healthy sensor may report the true value exactly. That does not remove the observation boundary: the sensor still has an update time, can fail, and can become unavailable. Flight fault logic must not bypass a failed sensor by reading plant truth.

## Minimal spacecraft model

### Power

Track battery energy in watt-hours, capacity in watt-hours, solar input in watts, and load in watts. Report state of charge as a percentage derived from energy and capacity. Choose and document any charging/discharging efficiency; unity efficiency is acceptable initially.

For a simple unity-efficiency model, battery-energy change is net power multiplied by elapsed simulation seconds, divided by 3,600. Clamp stored energy to the interval from zero to capacity and document that excess charging is discarded in this model. Negative elapsed time is invalid.

Account for at least an essential bus load and an optional payload load. Radio load may initially be a constant part of the bus load. Scenario sunlight controls available solar input. SAFE disables the payload, reducing load. Verify that at least one recovery scenario provides more charging power than SAFE consumes.

At zero energy, the baseline model remains a running software simulation in SAFE and reports depletion. It does not simulate electrical brownout, loss of processor power, or a reboot loop. State this limitation in the model and demo. A power-loss model is a later extension.

### Thermal behavior

Use one temperature state for the simulated electronics. Represent heat input from active loads and heat loss toward a scenario-defined environment. A first-order model with a selected thermal capacity and heat-transfer coefficient is sufficient. Document units, sign conventions, parameters, and integration step.

The important behavior is causal: payload operation should increase heating relative to the same scenario with the payload off; removing the load should reduce heating. SAFE does not instantly set a healthy temperature. Choose parameters that permit both a visible overheating scenario and a cooling/recovery scenario.

Check that your integration method remains stable at the selected step. Saturating an unstable or non-finite thermal value to a plausible temperature would hide a defect; surface a model error instead.

### Sensors and subsystem health

The baseline needs battery and temperature observations with a value, unit, validity/quality, and last successful sample time. A subsystem also has a reset count and a health state such as healthy, invalid, or timed out.

Support at least these injected conditions:

- A sustained physically low battery state.
- A sustained physically high temperature.
- A required sensor that stops producing successful observations.
- An explicitly invalid or non-finite observation.

An invalid observation must not overwrite the last good numeric value while retaining a “healthy” label. Preserve the last good value as historical context, mark its quality and age, and let required-observation timeout protection operate. Never substitute zero for missing data.

A sensor producing a plausible but incorrect constant value is a different fault from a missing sensor. Detecting that requires additional evidence and is an extension; do not claim baseline timeout logic detects it.

### Payload and orientation

The payload is a switchable source of load and heat. A command requests an absolute enabled/disabled state. It is enabled only in NORMAL and only after command validation. Disabling an already disabled payload is a valid no-op.

Orientation can initially be absent. If you add synthetic orientation, report its coordinate convention, units, and synthetic quality explicitly. Accurate orientation sensing and closed-loop attitude control are not first-release requirements.

## Time and execution

### Name every clock

| Clock | Purpose | Important constraint |
| --- | --- | --- |
| Simulation time | Physical evolution, sensor ages, fault persistence, spacecraft command validity | Advances according to the simulation runner; monotonic within a spacecraft boot |
| Host monotonic elapsed time | Real execution duration, scheduler lateness, local communication timeouts, UI freshness | Comparable only inside the process/clock domain that measured it |
| Wall-clock/UTC time | Human-readable log correlation and exported records | May be adjusted; do not use it to drive scheduler intervals |

Include run and spacecraft boot identity with simulation timestamps. A reboot may restart spacecraft simulation time; a new boot ID makes the discontinuity explicit. The scenario's overall timeline may continue across that reboot, but its mapping to the new boot must be recorded. A full scenario reset starts a new run.

Do not subtract a spacecraft monotonic timestamp from a ground monotonic timestamp. A packet's source time and its ground receipt time are different facts. Without a defined clock mapping or synchronization, ground cannot claim an exact end-to-end sample age.

### Two useful execution modes

**Deterministic model mode:** advance explicit simulation steps without requiring real-time sleeps. Given the same model version, configuration, ordered inputs, and random seeds, model state and discrete decisions should repeat within declared numeric tolerances. Stable ordering of simultaneous events is part of this promise.

**Interactive real-time mode:** pace the simulation so one simulated second approximately follows one elapsed second. Measure the actual relationship and report overruns. Use a declared policy for host stalls: bounded catch-up, slower simulation, or another explicit choice. A fixed-step model must advance by completed simulation steps, not by assumed sleeps that never occurred.

Simulation pause and speed changes are not spacecraft modes. They are optional scenario-runner controls. In the baseline acceptance runs, use continuous 1× pacing, keep the host awake, and treat suspension as a logged interruption rather than a spacecraft failure measurement. If pause is later exposed, show that the simulation is paused while ground's wall-time freshness continues to age.

### Suggested task profile

| Activity | Early implementation | Suggested mature profile | Time basis |
| --- | --- | --- | --- |
| Plant update and sensor sampling | 10 Hz is sufficient in Phase 1 | 100 Hz | Simulation releases; measure real execution separately |
| Health evaluation and protection | Added in Phase 2 | 10 Hz | Simulation time |
| Periodic status snapshot | Local report initially | 1 Hz | Simulation time |
| Command servicing | Local actions initially | Bounded work per iteration or explicit release | Defined by your executive |

These are adjustable targets, not reasons to invent a complex scheduler. Model integration and sensor sampling may use different rates if you document their relationship.

Measure scheduled releases, actual starts, completion durations, missed deadlines, and skipped work. Decide how missed releases are handled and place a bound on catch-up. A burst of old telemetry after a stall must not masquerade as fresh live status.

Run duration and jitter measurements in [06](06-verification.md) apply to the real-time runner. Fast deterministic model execution does not prove real-time scheduling performance. macOS measurements support a soft real-time claim within the tested conditions, not a hard real-time guarantee.

## Spacecraft modes

| Mode | Entry conditions | Expected behavior | Exit conditions |
| --- | --- | --- | --- |
| BOOT | New flight process/boot | Initialize identity and state, keep payload disabled, obtain required observations, start health evaluation and communications | NORMAL after startup checks pass; SAFE if checks fail or startup deadline expires |
| NORMAL | Successful boot or accepted recovery request | Required sensing, health evaluation, telemetry, permitted payload operations | SAFE on a qualifying protective condition or an accepted operator request |
| SAFE | Protection, startup failure, or operator request | Payload disabled; essential sensing, health checks, command reception, and telemetry continue | Explicit accepted request for NORMAL after recovery guards pass |

Startup checks must prevent an already unsafe initial observation from briefly enabling NORMAL. A suggested BOOT deadline is 5 simulated seconds; unavailable required observations at that deadline lead to SAFE with a startup reason. The runtime persistence timers below do not require BOOT to accept an unsafe initial condition first.

SAFE is latched: a recovered condition does not automatically restore NORMAL. Record entry reason, active faults, and which recovery conditions remain unmet. An operator-requested SAFE state can have no active physical fault; the operator request is still an explainable entry reason.

There is one logical authority for mode changes. A UI action cannot directly change the spacecraft mode. If a protective transition and an operator request contend, protection wins; command validation must use the current flight state at the point of effect. Record the resulting command outcome and mode event consistently.

## Fault detection and recovery

The following is a **suggested initial profile**, to be adopted or revised before Phase 2 acceptance. All durations in this table are simulation time. Separate entry and recovery thresholds provide hysteresis, avoiding rapid changes around one boundary.

| Condition | Qualification for active fault | Protective response | Recovery qualification |
| --- | --- | --- | --- |
| Low battery | Valid observed charge at or below 20% continuously for 2 s | Enter/stay SAFE; disable payload | Valid observed charge at or above 30% continuously for 5 s |
| Overtemperature | Valid observed temperature at or above 60 °C continuously for 2 s | Enter/stay SAFE; disable payload | Valid observed temperature at or below 50 °C continuously for 5 s |
| Required-observation timeout | Time since last successful required observation reaches 1 s | Enter/stay SAFE; disable payload | Valid fresh observations continue for 5 s; no gap reaches the timeout |

Evaluate protection no later than the next health release after qualification. At 10 Hz that adds up to 0.1 simulated seconds, assuming the health task runs as scheduled. Measure qualification-to-action separately from initial-disturbance-to-detection. Also report elapsed wall time for real-time runs so host overload cannot hide behind a stopped simulation clock.

Invalid observations break a healthy recovery interval and do not count as evidence that a low-battery or thermal fault has recovered. They also do not provide evidence of continuous hazardous readings; the timeout mechanism covers loss of usable observations. Document what happens to an unqualified persistence timer on invalid data; resetting it is the suggested baseline.

When a fault qualifies, record its first active time and the protective action. When its recovery guard qualifies, record it as cleared. Clearing an active fault and leaving SAFE are distinct events. A NORMAL request succeeds only when all required observations are valid and fresh, previously active faults have met their recovery guards, startup checks are complete, and no active protection condition remains. It must also reject currently hazardous readings even if their persistence interval has not yet qualified a fault. An unresolved condition makes the request fail with a reason.

Recovery hold intervals apply to conditions that were activated, rather than inventing physical faults for an operator-only SAFE request. For a startup failure, record which failed checks must pass before recovery; the suggested policy reruns the required-observation and safe-reading checks and honors any faults activated during BOOT. Document whether you select stricter NORMAL-entry guards than this baseline.

The first release must handle simultaneous faults: clearing a thermal fault must not conceal an outstanding low-battery or sensor fault. Repeated evaluation of the same active fault should update state/counters without producing an unbounded stream of identical events.

### What is not a spacecraft fault by default

A lost ground connection, rejected packet, or unavailable ground database is not automatically a reason for SAFE. The spacecraft continues its autonomous operations. Communication-error counters and ground alerts expose those conditions. A mission-specific loss-of-contact safe policy is an extension that would require a separate requirement and scenario.

A simulator's non-finite plant state or impossible configuration is an experiment failure, not a sensor reading to silently repair. Report it explicitly and stop or mark the run invalid according to your chosen runner policy.

## Local lifecycle and resets

Validate configuration before starting the run. Refuse impossible capacity, negative rates, inconsistent thresholds, or unusable endpoint settings with an actionable message. Do not begin a nominal-looking run with partially applied configuration.

On a deliberate shutdown, stop accepting new actions, terminate bounded work, and attempt to flush local diagnostic records. Ground must eventually mark spacecraft state stale; closing a socket is not a final authoritative spacecraft state. An abrupt termination can leave commands unresolved and a final log record missing.

A simulated sensor reset is distinct from a flight-computer restart. It increments that sensor's reset count and temporarily interrupts observations for a documented duration. A suggested reset duration is 2 simulated seconds, intentionally long enough to exercise timeout protection. It does not reset battery energy, temperature, run identity, or the flight boot ID.

A flight restart creates a new boot identity, clears volatile execution state, and returns through BOOT. Decide whether the plant survives independently or resets with that process. Both are valid learning models; document which one the demonstration uses. Do not describe a process restart that resets the plant as a realistic electrical reboot.

The core model does not retain durable “executed exactly once” command history across flight crashes. [03](03-interfaces-and-commands.md) specifies how boot identity and conservative ground reporting keep that limitation visible.
