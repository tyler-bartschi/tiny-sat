# 00 — Project charter

## Purpose

Build a small but credible spacecraft operations simulator that develops experience in C++, periodic execution, binary protocols, state machines, failure handling, distributed state, and operator tooling. Produce a portfolio project whose behavior you can demonstrate and whose engineering choices you can defend.

You should write the majority of the code. These documents define outcomes and constraints and suggest an order of work. They leave implementation decisions and the learning involved in making them to you.

## The mission being simulated

Imagine a small satellite with a battery, a solar charging source, temperature-sensitive electronics, a radio, and a switchable experimental payload. It has a flight computer that samples sensors, decides whether operation is safe, and sends status to an operator.

The payload can initially be an abstract electrical and thermal load. Enabling it increases power use and heat generation; disabling it reduces both. This gives commands a meaningful physical consequence without requiring an imaging or science simulation.

A scenario controls sunlight, initial conditions, disturbances, and injected failures. The operator can monitor the satellite and request permitted state changes, but the satellite must protect itself without a working connection to ground.

This is a simplified educational spacecraft model. Physical constants, thresholds, and scheduling targets are selected for understandable demonstrations. They are not a claim that this software or its model is suitable for controlling flight hardware.

## People and viewpoints

| Viewpoint | Needs |
| --- | --- |
| Developer | Small increments, inspectable state, reproducible scenarios, room to make design choices |
| Ground operator | Trustworthy status, visible uncertainty, traceable commands, clear recovery conditions |
| Scenario author | Controlled initial conditions, fault injection, link impairment, repeatable outcomes |
| Portfolio reviewer | Straightforward setup, a short demonstration, measured results, understandable limitations |

One person may fill all these roles. Keeping their permissions and information separate still makes the simulation meaningful: a scenario author can see model truth; the operator sees what arrived through telemetry.

## First-release scope

### Spacecraft side

- Central C++ flight application with a documented lifecycle.
- Simplified power and thermal evolution, plus sensors that can become invalid or stop updating.
- Periodic activities at multiple rates and measured execution timing.
- BOOT, NORMAL, and SAFE modes with explicit transition rules.
- Autonomous detection of sustained low battery, overheating, and required-sensor timeout.
- Commands to request a mode, set payload enabled/disabled, request current status, and reset a simulated sensor subsystem.
- Binary telemetry, command validation, command results, diagnostic counters, and events.

### Ground side

- A communication service in a language you select; Python or C++ are reasonable candidates.
- A React/TypeScript interface is the suggested UI direction, subject to your decision.
- Persistent telemetry, command history, and events, with a basic query/export path.
- Current status, limited historical trends, freshness indicators, fault/recovery explanations, and command outcomes.
- Clear handling of an absent spacecraft, a disconnected browser, or unavailable storage.

### Integration

- Flight and ground run as separate local processes.
- A logical link impairment boundary exists in each direction; it may initially be embedded in an endpoint rather than a third process.
- Deterministic model scenarios and repeatable impairment inputs support meaningful verification.
- Restart, overload, malformed input, and partial failure have defined behavior.

## First-release exclusions

Keep these out of the critical path: orbital propagation, realistic attitude control, reaction wheels, RF modulation, an antenna controller, multiple spacecraft, multiple commanding operators, internet exposure, mission scripting languages, full telemetry replay into the live system, software updates in flight, and durable command execution across spacecraft power loss.

You may add an explicitly synthetic orientation display for visual interest. It must be labeled as such and must not imply that an attitude-control system exists.

The core release does not implement an actual spacecraft communications standard. A custom protocol is intentional learning scope. Interoperability with existing ground tools is a later project decision.

## What makes this more than a dashboard

A battery gauge alone is presentation. The engineering work is making its meaning reliable: the value has a source time, a quality state, a spacecraft boot identity, and an age at the ground station. A successful button click alone is UI behavior. A successful command requires validation, execution evidence, and a response whose identity survives retries.

The project should let you explain questions such as:

- What does the spacecraft do when no ground station is running?
- If an acknowledgment is lost, can ground tell whether the command happened?
- Can an old packet make the UI show an earlier spacecraft state as current?
- Can a stale command from before a restart affect a newly booted spacecraft?
- How quickly is a qualifying fault detected, and why does it clear when it does?
- What happens if the ground database stops accepting writes?

## Success criteria

The first release is complete when the baseline requirements in [06](06-verification.md) have evidence, the selected numeric targets are documented, and a reviewer can reproduce a mission demonstration using the repository's instructions.

A strong demonstration includes normal operation, a payload command, a lost or delayed result, a communication outage, autonomous safe-mode entry, reconnection, and an explicit recovery attempt. It shows the resulting timeline and explains what ground could and could not know at each stage.

Repository organization, UI polish, and a long feature list do not substitute for these behaviors. Conversely, a compact model with clear invariants, measured limits, and well-explained choices is an appropriate finished project.

## Keeping scope under control

Use phases as completion gates, not calendar promises. Do not assign a fixed schedule until you have completed an increment and learned your pace. Record attractive new ideas in the expansion backlog. Add them to the core only by explicitly revising scope, prerequisites, and verification expectations.

When a design question blocks progress, make a small, reversible decision and record its assumption. A prototype can answer a specific uncertainty, but keep it clearly distinguished from a verified capability.
