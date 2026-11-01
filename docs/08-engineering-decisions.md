# 08 — Engineering decisions you own

## Purpose

The specification defines required behavior. Your design explains how the implementation produces it. Record decisions that affect boundaries, correctness, significant complexity, or future changes. Avoid documenting every minor naming choice.

Use short architecture decision records (ADRs). Add them under `docs/decisions/` when you make real choices. Accepted choices are recorded only after you make them.

## Accepted decisions

- [001 — Keep the project in one repository](decisions/001-single-repository.md). All components initially run on the Mac, with future Raspberry Pi deployment left open. Folder and build organization remain undecided.

## Decision register

All entries below are **open**. The proposed starting points elsewhere in the specification are recommendations, not recorded selections.

| Topic | Resolve by | Questions to consider |
| --- | --- | --- |
| Folder and build organization | Phase 0 | How are responsibilities separated inside the chosen single repository? How can each component build without unrelated dependencies? |
| Toolchain and test approach | Phase 0 | Which supported C++ tools work locally? What does a fresh machine need? |
| Time and deterministic stepping | Phase 0 | What advances model time? How are steps, releases, host stalls, and reboot discontinuities related? |
| Plant/sensor/flight ownership | Phase 0 | Who can read truth, publish observations, request effects, and commit mode changes? |
| Initial model fidelity and parameters | Phase 0 | What is physically represented, approximated, or deliberately absent? |
| Scheduling and concurrency | Phase 2 | What can block? Who owns shared state? What work can be skipped? |
| Fault qualification and recovery | Phase 2 | Which observations qualify faults? What happens at boundaries or on invalid data? |
| Mode/action arbitration | Phase 2 | How do commands and protection converge on one consistent resulting state? |
| Transport and wire encoding | Phase 3 | How are frames bounded and validated? What failure behavior is visible? |
| Protocol identity and compatibility | Phase 3, harden in 5 | How are commands, attempts, boots, streams, wrap, and retired sessions distinguished? |
| Ground-service stack and API | Phase 3–4 | What does the browser need? Where is authoritative validation? |
| Storage, durability, and retention | Phase 4, harden in 6 | What survives restart? How much data is retained? What happens if writes fail? |
| UI freshness and history | Phase 4–5 | Which clock supports each age claim? How are gaps, late samples, and uncertainty shown? |
| Command retry/expiry/deduplication | Phase 5 | What is safe after cache eviction or a lost result? Which clock enforces validity? |
| Impairment reproducibility | Phase 5 | What is seeded? What ordering is recorded? Where is corruption applied? |
| Restart/reconciliation | Phase 5 | What survives each restart? Which pending actions can be resolved, and which remain unknown? |
| Resource limits and shutdown | Phase 6 | What are the capacities, rejection policies, wall-time bounds, and flush limits? |
| Release packaging and claims | Phase 6 | What can a reviewer reproduce, and what do the measurements actually support? |

“Resolve by” allows revisiting a decision later. Record the replacement and what caused it rather than editing away the original rationale.

## Suggested ADR format

Use an identifier and descriptive title, then include:

1. **Status and date:** proposed, accepted, or superseded.
2. **Context:** the problem and relevant constraints, including linked requirements.
3. **Alternatives:** two or three plausible options and their material tradeoffs.
4. **Decision:** what you selected and why it fits the current project.
5. **Consequences:** useful properties, added complexity, and known limitations.
6. **Verification impact:** checks, selected targets, or fixtures that establish the choice works.
7. **Revisit trigger:** evidence or future requirements that would justify changing it.

A small decision can fit in half a page. If an experiment is needed, state the question and pass condition before trying alternatives.

## Questions to answer in your own design

### For the flight executive

- What state can change during a health check, and can telemetry observe a partial transition?
- How is command validation kept consistent with application of its effect?
- Can a slow operation monopolize the executive? What is the bound?
- After a host stall, does the simulation catch up, slow down, or skip releases?
- Which counters reveal that the apparent nominal rate was not actually maintained?

### For the protocol

- Can independently implemented endpoints agree on bytes without sharing one buggy serializer?
- What can be validated before trusting a length, message type, or identity?
- When results arrive out of order, can a terminal command record move backward to accepted?
- After deduplication-result eviction, why is an old request still unable to repeat an effect?
- How does a receiver distinguish an old boot's delayed traffic from a newly established boot?

### For ground control

- What does “current” mean, and which clocks make that claim supportable?
- What does the operator see when the database is reachable but the spacecraft is not, or vice versa?
- Which records explain uncertainty rather than hiding it?
- What happens to an in-progress request if only the browser restarts?
- How can a reviewer tell observed spacecraft state from scenario-author knowledge?

## Keeping changes coherent

When changing a requirement or a design choice, identify affected behavior, interfaces, phases, and verification scenarios. Update only the documents that actually change. Preserve stable requirement IDs; retire an ID rather than reusing it for an unrelated behavior.

Distinguish a target change from a failed result. It is reasonable to discover that a numeric goal is a poor fit. Preserve the measurement, explain the new goal, and re-run the relevant check. This is more informative than silently moving a threshold.

Once implementation exists, add a concise architecture document showing your actual module/process boundaries and the reasons behind them. These initial specifications should remain a behavioral reference rather than pretending the proposed diagram is your final design.
