# tiny-sat: project specification and development guide

**Status:** proposed specification, revision 0.1, October 30, 2026. Implementation has not started. The documents describe intended behavior, not capabilities already built.

tiny-sat is a local spacecraft operations simulator: a C++ flight computer operates a small simulated spacecraft while a separate ground-control system observes telemetry and sends commands across a configurable communication link. The developer writes the implementation and owns the design decisions.

The first complete release should let an observer watch normal operations, issue a command, interrupt communications, inject a spacecraft fault, and understand the spacecraft's autonomous response and subsequent recovery from recorded evidence.

## How to use these documents

Read the charter and Phase 0 first. Skim the behavior and interface specifications before choosing your initial design. Then use the phase guide as your working roadmap and the verification document as your definition of completion.

| Document | What it answers |
| --- | --- |
| [00 — Project charter](00-project-charter.md) | What am I building, for whom, and where does the first release stop? |
| [01 — Phase 0 and repository setup](01-phase-0.md) | How should I organize the work, and which decisions need an early answer? |
| [02 — Spacecraft behavior](02-spacecraft-behavior.md) | What do the simulated spacecraft and flight computer do? |
| [03 — Interfaces and commands](03-interfaces-and-commands.md) | What crosses system boundaries, and what do messages mean? |
| [04 — Ground control](04-ground-control.md) | What can an operator see, command, and recover after an interruption? |
| [05 — Development phases](05-development-phases.md) | In what order can I build useful, demonstrable increments? |
| [06 — Requirements and verification](06-verification.md) | How do I know each capability works, and what evidence should I keep? |
| [07 — Expansions](07-expansions.md) | What can I add once the core system works? |
| [08 — Engineering decisions](08-engineering-decisions.md) | Which choices remain mine, and how can I record the reasoning? |
| [09 — Glossary and references](09-glossary-and-references.md) | What do the terms mean, and where can I learn more? |

## What is fixed and what is yours to choose

**Baseline requirement:** a behavior required to call the proposed first release complete. The requirements register assigns it an identifier, an introduction phase, and verification scenarios. Earlier phases are deliberately partial implementations.

**Suggested target:** a proposed numeric value or operating profile, such as a 100 Hz task or a 3-second stale-data threshold. These are educational project choices, not aerospace standards. Adopt them or record a reasoned alternative before running the relevant acceptance check. Record the selected values with the result so that “passed” has a definite meaning.

**Recommendation:** a starting approach that you may replace after considering its tradeoffs.

**Decision:** something you must resolve as the designer. These documents do not prescribe class hierarchies, thread counts, exact packet layouts, libraries, or a database schema.

**Extension:** an optional capability outside the first-release completion criteria.

Requirement IDs and the verification mapping live in [06 — Verification](06-verification.md). The other documents explain their intent and edge cases. If two documents disagree, resolve and record the discrepancy before implementation; do not silently select the easier interpretation.

## The minimum complete project

- A C++ flight application operates power and thermal models, reads simulated sensors, schedules periodic work, and manages BOOT, NORMAL, and SAFE modes.
- The spacecraft continues operating when ground software is absent.
- Separate flight and ground processes exchange versioned binary commands, telemetry, and command results with integrity checks and sequence information.
- Ground control provides live status, trends, command history, explicit data freshness, and persistent operational records.
- A controlled link model can delay, lose, corrupt, duplicate, reorder, and interrupt messages in either direction.
- The system demonstrates bounded fault response, conservative command handling, reconnection, and honest reporting of uncertainty.
- Repeatable scenarios and recorded measurements substantiate the behavior.

An initial orientation field may be unavailable or explicitly synthetic. Actual attitude dynamics, orbital mechanics, RF hardware, cloud deployment, and Raspberry Pi deployment are extensions.

## A manageable first move

Complete the small Phase 0 decision set, then aim for the Phase 1 demonstration: a local C++ process whose battery and temperature change over time and whose state you can explain. You do not need to settle the whole project before producing that first result.

The repository currently contains this specification only. Proposed directories and tools are planning guidance; no runtime, build system, or launch command is supplied yet.
