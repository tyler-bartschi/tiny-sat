# 09 — Glossary and learning references

## Working vocabulary

| Term | Meaning in this project |
| --- | --- |
| Plant | The simulated physical spacecraft: energy, temperature, loads, and environmental response |
| Plant truth | Internal physical-model state, accessible to simulation/verification rather than ordinary flight decisions |
| Observation | A sensor-provided value with time, units, and quality |
| Flight executive | The part of the flight application that coordinates scheduled work and state transitions |
| Subsystem | A bounded spacecraft responsibility, such as power monitoring or a temperature sensor |
| Mode | A spacecraft-wide operating policy: BOOT, NORMAL, or SAFE |
| Telemetry | State/measurements sent from spacecraft to ground |
| Command | An identified request for the spacecraft to perform a supported action |
| Uplink / downlink | Ground-to-spacecraft / spacecraft-to-ground message direction |
| Command acknowledgment/result | Evidence of a defined stage such as acceptance, rejection, or completion; its exact meaning must be explicit |
| Idempotent action | Repeating the same request does not create an additional effect beyond the intended state |
| Deduplication | Recognizing repeated command identities so one request does not repeat an effect |
| At-most-once effect | Within the declared scope, a command effect happens zero or one times; delivery/completion is not guaranteed |
| Outcome unknown | Ground lacks evidence sufficient to determine whether the intended effect completed |
| Boot identity | Identifier for one flight-computer execution lifetime; distinguishes traffic across restarts |
| Origin session | Identifier for a ground command-origin lifetime, preventing identity reuse across restarts |
| Run identity | Identifier for one configured simulation experiment, which may contain multiple boots |
| Full snapshot | Message sufficient to establish the current reported state without receiving earlier snapshots |
| Freshness | A stated age/progress property of received data, qualified by clock domain and field quality |
| Hysteresis | Different entry/recovery thresholds to avoid oscillating around a single threshold |
| Persistence / hold interval | Time a condition must remain true before qualification or recovery |
| Release | Scheduled point at which a periodic activity becomes due |
| Start lateness | Actual start time minus scheduled release time, in the same clock domain |
| Execution duration | Elapsed time from activity start to completion |
| Deadline miss | An activity fails to complete by its declared deadline; skipped required work also counts as missed work |
| Jitter | Variation in timing; specify whether this means task timing or injected link-delay variation |
| Monotonic clock | Clock used for elapsed intervals without wall-clock adjustments; not directly comparable across arbitrary processes |
| Soft real-time | Timing matters and is measured, but deadline compliance is not guaranteed under all execution conditions |
| CRC | Cyclic redundancy check used here for accidental-corruption detection, not sender authentication |
| Golden fixture | Known input/message with independently specified expected output/bytes |
| Fault injection | Deliberately causing a model, sensor, link, or service failure to examine behavior |
| Reconciliation | Re-establishing a supported present view and resolving historical outcomes where evidence permits |
| ADR | Architecture decision record: context, alternatives, selection, consequences, and verification impact |

## Primary references

These references informed specific concepts and offer further reading. They do not impose their frameworks, standards, or process requirements on tiny-sat. The selected numeric profiles throughout this specification are original project suggestions.

### Measurable requirements and verification

[NASA Software Engineering Requirements, Chapter 4](https://nodis3.gsfc.nasa.gov/displayDir.cfm?Internal_ID=N_PR_7150_002D_&page_name=Chapter4) discusses clear, consistent, individually verifiable requirements and their relationship to design and validation. For this project, borrow the discipline of connecting a behavior to evidence while keeping the documentation proportional to a solo learning project.

### Periodic flight activities

[F Prime v3.6.0: Rate Groups and Timeliness](https://fprime.jpl.nasa.gov/v3.6.0/docs/user-manual/design/rate-group/) provides a concrete example of periodic activities grouped by frequency. Use it to learn terminology and compare design options. Its component model and exact scheduling implementation are not requirements for your executive. The link is version-specific so the reference remains identifiable.

### Transport limitations and reliability

[RFC 8085, Section 3.3: UDP Reliability Guidelines](https://www.rfc-editor.org/rfc/rfc8085.html#section-3.3) explains that UDP does not provide reliable delivery, ordering, or duplicate protection. It is a useful basis for deciding which behavior your application must handle. [Section 3.4](https://www.rfc-editor.org/rfc/rfc8085.html#section-3.4) covers checksum considerations. The RFC is broader than the local simulator; tiny-sat's loopback experiments do not establish suitability for arbitrary internet deployment.

## Suggested learning order

Begin with the unfamiliar concept needed for the next phase rather than reading an entire aerospace software ecosystem first:

1. Model units, discrete time steps, and simple state transitions for Phase 1.
2. Monotonic timing, periodic releases, fault qualification, and state ownership for Phase 2.
3. Explicit byte encoding, integrity, bounds checking, and command correlation for Phase 3.
4. Source versus receipt time, persistence, and a minimal UI/service contract for Phase 4.
5. Idempotency, uncertainty, replay rejection, and session reconciliation for Phase 5.
6. Measurement methodology, overload, reproducibility, and honest release claims for Phase 6.

Consult official documentation for the compiler, libraries, database, and UI stack you actually select. Tool versions and platform instructions belong in the implemented project's verified setup guide, not in assumed commands written before those choices exist.
