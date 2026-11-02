# 007 — Initial ideal power model and incremental thermal expansion

**Status:** accepted model scope, November 1, 2026. Numerical settings and implementation details listed below remain open.

## Context

Simulation owns physical state and simulation time, runs independently of flight, and applies actuator changes at simulation-step boundaries under [decision 003](003-simulation-and-flight-timing.md). It communicates with flight under [decision 004](004-simulation-flight-tcp.md). Implementation uses the C++20/CMake toolchain in [decision 006](006-cpp-and-shared-cmake-build.md).

The developer wants an approachable first physical model that can be independently checked, then extended. Plan for power and thermal behavior, implement and verify power first, and add temperature in the following increment. This selects the first runnable slice rather than removing thermal behavior from the planned spacecraft.

## Accepted decisions

### 1. Begin with an ideal energy store

Represent the battery through a fixed energy capacity and current stored energy. Initially assume lossless charging/discharging and no separate charge or discharge power limit. A nonempty battery can cover a generation deficit until its available energy is exhausted.

Voltage/current behavior, charging electronics, battery chemistry, degradation, self-discharge, and temperature-dependent battery performance are deferred. A power limit that prevents a charged battery from meeting a large demand would be an additional model capability, not something inferred from energy capacity.

### 2. Use explicit units and one authoritative stored-energy value

- Battery energy and capacity: watt-hours (Wh).
- Generation and requested load: watts (W).
- Elapsed simulation intervals: seconds.
- State of charge: percentage derived from stored energy divided by capacity.

Do not maintain charge percentage as an independently evolving physical state. The model must keep stored energy between zero and capacity. Capacity is positive; loads and available generation are nonnegative. Exact validation/error behavior and numerical tolerances remain to be defined.

For a constant-power interval before a boundary is reached, the ideal energy change is net charging power multiplied by elapsed simulation seconds, divided by 3,600. The model must account for full/empty boundaries rather than assume the unconstrained change can always occur.

### 3. Start with one generation input and two loads

Use one configurable available-generation power input, one constant essential load, and one switchable constant payload load. Total requested load is essential demand plus payload demand while the payload is enabled.

Scenarios can change available generation, including setting it to zero. Initially supply that input directly in watts; solar-panel geometry, orbit-derived sunlight, and detailed source electronics are deferred. The exact scheduling of scenario changes is still to be selected.

Payload enabled/disabled is applied hardware state controlled through the existing actuator interface. This initial model does not automatically disable the payload because of insufficient supply.

### 4. Use generation first, then balance with the battery

Available generation supplies requested load first. Surplus charges the battery until capacity is reached. A deficit draws from stored energy until the battery is empty. Requested energy that cannot be supplied becomes unmet demand.

The ideal model has no simultaneous net charge/discharge requirement: the generation/load balance determines the battery's direction of energy flow. No device-level bus allocation or electrical-circuit solver is selected.

### 5. At full capacity, account for curtailed surplus

Do not store energy above capacity. Continue supplying loads from available generation and account for unused surplus as curtailed available-input energy. A full battery can subsequently discharge if demand exceeds generation.

Curtailment is an explicit accounting outcome, not an automatic heat source in the future electronics thermal model. Hardware that dissipates excess power as heat would require an additional physical assumption.

### 6. At empty, report unmet demand without equipment power loss

Do not allow negative stored energy. Available generation may still supply some or all requested demand at zero battery energy; any surplus can begin charging the battery.

Report the portion of demand that available generation and battery energy cannot supply. For this initial increment, do not shut down devices, stop the flight process, initiate an electrical reboot, or alter payload enabled state automatically because supply is insufficient. The simulation continues evolving.

This deliberately leaves equipment power-loss behavior unmodeled. It does not assert that a real computer could run correctly on a fractional electrical supply. Later flight protection may request load changes while running; that is distinct from modeled hardware power-loss enforcement.

Unmet demand is aggregate accounting. Do not claim which individual device received power or assign essential/payload shedding priorities. Such allocation and power-loss/restart behavior are future decisions.

### 7. Account for reaching a boundary during an interval

If a battery fills or empties partway through a modeled interval, distinguish the energy actually stored/supplied from curtailed or unmet energy during the remainder. Clamping final battery energy alone is insufficient accounting.

For example, with constant inputs, a battery may cover a deficit for only part of the interval. Do not report the whole interval as fully supplied or fully unsupported. The calculation method and any internal subdivision are implementation decisions; this does not change the step-boundary policy for external actuator application.

### 8. Preserve interpretable energy accounting

Expose enough model results to distinguish stored energy, requested demand, supplied load energy, unmet demand, and curtailed input energy. Preserve the interval or accumulation scope associated with each quantity. Exact internal structures, telemetry fields, and output formats remain open.

For a lossless interval, check:

**Available input energy + starting battery energy = supplied load energy + ending battery energy + curtailed input energy.**

Also check:

**Requested load energy = supplied load energy + unmet demand energy.**

These are model-accounting identities, with numerical tolerances still to be selected. Unmet demand is requested energy never supplied, rather than another energy source or stored-energy loss.

### 9. Anticipate thermal behavior without implementing it first

Keep power and thermal responsibilities distinguishable and let both use consistent simulation time, environmental inputs, and applied payload state. Add a simple electronics temperature model after the power increment is verified.

When thermal behavior is introduced, decide how supplied electrical energy becomes heat and how supply shortages affect that calculation. Do not automatically interpret requested load as fully delivered power during an unmet-demand interval. Thermal equations, parameters, and any per-device allocation remain open.

No general subsystem framework, class hierarchy, or shared model library is required by this decision.

## Alternatives and tradeoffs

Implementing power and temperature together would demonstrate coupled behavior earlier but introduce more uncertain parameters and potential fault sources at once. Power-first implementation provides independently calculable cases and a tested base for later integration.

Detailed electrical modeling or immediate equipment shutdown could increase realism, but would require additional constraints and lifecycle policies. The selected ideal model prioritizes units, energy balance, bounded storage, and explicit limitations before those extensions.

## Verification intent

Use independently calculated expectations for constant charging, constant discharging, and balanced generation/demand. Exercise zero/full boundaries and boundary crossings within intervals. Verify surplus curtailment, unmet-demand accounting, payload-on/off demand changes, and generation restoration after depletion.

Check both energy-accounting identities, battery bounds, and derived state of charge. Include distinct cases where an empty battery has sufficient generation, partial supply, or no generation. Verify that shortages do not silently change device enable state or imply unimplemented load allocation.

Use repeatable initial conditions and selected numerical tolerances. Catch2 is the chosen C++ test framework; no test code or runtime implementation is created by this record. When temperature is added, preserve the existing power expectations and add coupled scenarios.

## Remaining decisions

- Reference capacity, initial stored energy, generation, essential load, and enabled-payload load values.
- Configuration representation, invalid-input handling, and numeric tolerances.
- Fixed/variable stepping policy and step size, plus assignment of scenario changes to simulation intervals.
- Instantaneous versus interval-averaged versus cumulative reporting conventions.
- Model/runner interfaces and implementation organization.
- Later thermal equations and parameters, electrical power limits, per-device supply allocation, and equipment power-loss/restart behavior.

## Relationship to the initial specification

The original [spacecraft behavior specification](../02-spacecraft-behavior.md) proposes power and thermal capabilities for Phase 1 and describes a depleted-battery simulation limitation. This record clarifies power as the first increment, keeps thermal in the planned scope, and makes energy shortages/curtailment explicit. It does not direct the simulator to change flight mode at zero energy; flight mode remains a flight-software decision.

Other specifications, decisions, and code are unchanged. Any reconciliation edits require the developer's explicit request.

## Revisit triggers

Revisit when thermal coupling, fast electrical behavior, power-limited devices, load shedding, or physical flight-computer power loss becomes a selected learning objective.
