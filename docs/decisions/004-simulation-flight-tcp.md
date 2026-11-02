# 004 — Simulation–flight TCP connection and request recovery

**Status:** accepted architecture, November 1, 2026. Detailed encoding, limits, and mechanisms listed below remain open.

## Context and scope

Simulation and flight run as separate processes on the developer's Mac. Simulation owns physical state and time and continues independently of flight, as recorded in [decision 003](003-simulation-and-flight-timing.md).

This decision concerns access to simulated sensors and actuators. It does not select the flight-to-ground transport or require this interface to use the eventual binary ground protocol.

## Accepted decisions

### 1. Connection roles and lifetime

Use TCP on the local Mac, with simulation listening and one controlling flight instance connecting. Use one persistent, bidirectional connection for session exchange, observations, actuator requests/results, result acknowledgments, and capacity notifications.

Simulation continues physics when flight is absent, disconnected, or unresponsive. Communication must not indefinitely block physical progression. The initial deployment is local; Raspberry Pi/network deployment details are deferred.

Only one flight controller may be active at a time. The exact handling of a second connection, including takeover or replacement of an existing connection, remains open.

### 2. Explicit, bounded framing

Define application message boundaries and enforce agreed size/resource constraints. A length prefix is a candidate, not a selected representation. Encoding, prefix format, byte order, versions, and exact limits remain open.

TCP supplies an ordered byte stream; a send or receive operation does not define one complete application message. TCP transport receipt is also distinct from flight processing an actuator result. See the [TCP specification](https://www.rfc-editor.org/rfc/rfc9293.html).

### 3. Sensor observations favor the latest sample

Simulation publishes observations at appropriate per-sensor intervals. Retain at most one pending, unsent application observation per sensor: the latest one replaces the previous unsent observation for that sensor.

This limit applies to application-owned pending observations. It cannot replace bytes already submitted to the TCP stream. The eventual implementation must also bound transport submission/backlog and preserve source time, sequence, and quality information. Sampling rates, delivery fairness, and reconnect snapshot details remain open.

### 4. Request and result identity

Correlate each logical actuator request and its result using the combination of:

- **Simulation run identity:** distinguishes one experiment from another; remains stable across flight reconnects/restarts within that experiment.
- **Originating flight-instance identity:** remains stable when the same flight process reconnects; changes when flight restarts.
- **Request identity:** distinguishes logical requests from that flight instance; remains unchanged when retrying the same request and must not be reused for another request within its scope.

Together these distinguish requests across retries, flight lifetimes, and simulation experiments. Identity generation, representation, widths, and sequence/wrap rules remain open. An operating-system process identifier alone is not a sufficient non-reuse policy.

A timestamp or matching request contents alone does not establish duplication. Different legitimate requests may share a simulation timestamp or request identical actions.

For the same composite identity and the same semantic request contents, recognize a retry and do not repeat the effect. If still in progress, preserve that work rather than enqueue it again. If completed and its result is retained, return that result.

For the same composite identity with conflicting semantic contents, report a protocol/identity conflict without applying another effect or overwriting the original result. The exact conflict-message representation remains open.

Transport reconnection or a new transmission attempt does not create a new logical request. A new intentional action receives a new identity, even if its requested state matches an earlier action.

### 5. Three terminal actuator statuses

| Status | Meaning |
| --- | --- |
| Rejected | Simulation declined the request before beginning its effect, such as a known hardware inability to perform it |
| Failed | Simulation admitted the request, but the operation did not complete successfully |
| Completed | The requested change completed successfully |

Here, **Completed means completed successfully**, not merely received or queued. Use a distinct internal term such as admitted for work awaiting execution. Actuator effects occur at simulation-step boundaries under decision 003.

Result details must make the outcome interpretable, including a reason and available application-time/resulting-state information as appropriate. Failed work must not imply that no partial effect occurred. Exact fields are still to be designed.

Malformed/unidentifiable frames, session errors, and capacity notifications are separate protocol concerns. They do not require inventing a terminal actuator result for work that was never admitted.

### 6. Retain results until application acknowledgment

For an admitted request, retain its terminal result regardless of whether the status is Rejected, Failed, or Accepted. Send it to flight and retain it until flight acknowledges that identified result at the application level.

The ordinary successful exchange has three application messages: request, terminal result, and result acknowledgment. These are not necessarily three TCP packets.

If disconnected before acknowledgment, keep the result and retransmit it on a validated reconnection. Sending bytes or receiving TCP transport acknowledgments alone does not retire it. Flight must tolerate duplicate results and acknowledge them again without treating them as new actions. Duplicate result acknowledgments must be harmless.

Acknowledgment retires the retained response record. It does not remove the retention policy or justify forgetting all execution history. Keep sufficient replay-tracking metadata to prevent an acknowledged old request from executing again if redelivered. Choose a bounded retention/window/floor mechanism before implementing eviction; exact storage algorithms remain open.

### 7. Bounded capacity and admission notifications

Bound retained actuator results and outstanding admitted work. Reserve capacity for the eventual result when admitting a request, including a request that will be rejected, so concurrent completions cannot overfill retention.

When capacity is exhausted, pause admission of additional actuator requests while physics continues. Already admitted work retains its result reservation. Use capacity notifications to tell flight that admission is paused or available again, rather than creating another retained rejection for every excess request.

Capacity notifications are connection-control messages with a separate bounded/coalescing policy. They are not retained actuator results in the three-message acknowledgment lifecycle. Their exact representation and delivery policy remain open. Simulation must enforce its admission limit even if a notification has not yet reached flight.

Paused actuator admission must still permit session/control processing and result acknowledgments needed to release capacity. Do not implement it as an indefinite wait in the physics path or a blanket stop to all connection reading. The protocol still needs a concrete rule for excess request frames already in transit and for a nonconforming sender; such frames cannot cause an unbounded ingress queue or unreserved actuator execution.

This guarantee covers admitted actuator requests. It does not promise a retained per-request result for unlimited work submitted while admission is paused. Reserve all relevant identity/replay resources as well as result storage, with bounded exhaustion behavior.

### 8. Session validation and flight restart recovery

Exchange information and validate a session before operational traffic is admitted. Establish compatible protocol interpretation, the current run identity, and the connecting flight-instance identity. Additional state exchange is to be specified later. These identities provide correlation and lifecycle boundaries; they do not by themselves authenticate a process.

The same flight process reconnects with the same identity, allowing retained results to return to it. A restarted flight process uses a new identity. Simulation sends all retained results from the current experiment to the new controlling instance as recovery information, including results originating from previous flight instances.

Historical results retain their original run, flight-instance, and request identities. Do not relabel them as responses to new requests. Results originate from simulation and correlate to the flight instance that requested the action; the current connection is a delivery destination, not a replacement origin.

The new controller decides how to handle historical data and may acknowledge those historical results after handling them. They must not be mistaken for its new requests or automatically re-executed. Simulation must retain old origin information on pending work/results and replay metadata for as long as required by the chosen safety policy, even after the active controller identity changes.

### 9. Simulation restart starts a new experiment

A simulation restart ends the old experiment and begins a new one. Simulation, flight, and ground are restarted/reset into the new experiment, rather than resuming the old active session. The exact orchestration mechanism remains open.

Use a new simulation run identity and new flight lifetime. Old-run requests, results, and acknowledgments must not be interpreted as new-run traffic. Persisted records, if any, remain historical rather than active work. This design does not promise continuation of an old request/result lifecycle across simulator restart.

### 10. Simulation initially uses one event loop with bounded work

**Accepted:** November 1, 2026.

Use one simulator thread/event loop to coordinate due physics steps, eligible actuator application, sensor publication, TCP communication, session handling, and request/result lifecycle state. The simulator's physical state has one execution owner. This decision applies to simulation; flight's internal execution model remains open.

Use nonblocking TCP socket operations. An absent peer, incomplete message, unavailable send-buffer space, or missing acknowledgment must not hold the event loop waiting for flight. Retain incomplete receive frames and outgoing transmission progress so the loop can return to them later. Once an outgoing frame has begun transmission, keep that frame stable until it finishes; newer observations may replace the next unsent observation, not bytes in a partially transmitted frame.

Bound work per loop turn across receiving, framing/parsing, request/control processing, sending, and physics catch-up. A busy connection must not monopolize execution, and catch-up must leave opportunities for acknowledgments, session processing, and other communication. Do not drain an unlimited stream of input or perform an unlimited catch-up loop before reconsidering due work. Actual byte/message/time budgets and fairness policy remain to be selected.

When there is no immediately eligible work, the loop may wait for relevant socket readiness with a timeout bounded by the nearest scheduled activity. It must not wait indefinitely for TCP activity when a physics or other timer deadline is pending. Use the elapsed-time clock and pacing/critical-lag policy from decision 003. The readiness API or event-loop library is not selected here.

Nonblocking TCP alone is not sufficient: model calculations, parsing, diagnostics, and other synchronous work must also fit the timing budget. A single event loop cannot preempt a long-running calculation inside its own thread. Measure overruns and lag rather than assuming this structure guarantees deadlines. Exact logging behavior, numerical workload limits, and catch-up parameters remain open.

Avoid busy-waiting continuously on an idle connection. Maintain explicit unfinished-work state and bounded buffers, and resume operations when ready or when a relevant timer requires service. A completed socket send still does not retire an actuator result before application acknowledgment.

This choice avoids cross-thread handoffs initially. It does not select class structure, a socket wrapper, a framework, or fixed numeric timing values.

## Alternatives and consequences

TCP was chosen over a local-only transport for the initial sensor/actuator connection. One connection keeps lifecycle handling compact, while requiring bounded submission and scheduling so observation traffic does not indefinitely delay results or controls.

Keeping latest unsent observations sacrifices delivery of superseded samples. Retaining terminal results until acknowledged preserves recoverability while simulation survives, but requires admission limits, stable identity, duplicate handling, and historical-result acknowledgment after flight restart.

Capacity notifications provide bounded overload signaling. They do not eliminate the need for simulation to enforce limits independently or for flight to track unresolved requests.

A dedicated communication thread could process network activity during a physics calculation, but would require synchronized handoffs, additional ownership rules, and coordinated shutdown. The single event loop was selected initially for straightforward state ownership and traceable ordering. Its cost is cooperative scheduling: all work must return promptly enough for other responsibilities to progress.

## Verification intent

- Start simulation with no flight connected; physics continues. Connect flight later and establish a valid session.
- Disconnect the same flight instance and reconnect it; retained results retain their identities and are delivered again.
- Restart flight; historical results reach the new controller with old origin metadata intact and can be acknowledged.
- Deliver duplicate requests while work is pending, after completion, and after result acknowledgment; no repeated effect occurs within the supported replay policy.
- Exercise identity/payload conflicts and legitimate same-timestamp requests with distinct identities.
- Lose a result acknowledgment; retransmission and repeated acknowledgment do not create another action.
- Fill capacity, continue physics, pause admission, deliver capacity information, process acknowledgments, and resume admission without exceeding bounds.
- Stall reception and verify latest-per-sensor application buffering and bounded connection submission.
- End/restart the experiment; old-run traffic cannot enter the new run as operational work.
- Leave flight absent/idle, deliver fragmented frames, and stall its reception; physics continues while the event loop preserves bounded partial-message state.
- Offer sustained traffic and force catch-up; verify per-turn work limits, continued control/acknowledgment processing, and measured physics pacing or explicit critical-lag termination.
- Exercise partial writes while newer observations arrive; complete frames remain intact and the latest unsent per-sensor observation is preserved.
- Exercise idle/timer waiting and a deliberately slow operation; verify no idle busy-spin and that lateness/lag is measured rather than hidden.

These are intended checks, not implemented tests or passing results.

## Remaining protocol and implementation decisions

- Encoding, framing details, versions, limits, and parsing/error behavior.
- Identity generation, request semantic comparison, replay-window retention, and acknowledgment format.
- Session handshake state fields, initial snapshots, readiness, and second-connection handling.
- Exact result capacity and pending-work limits; capacity-notification and in-transit overflow rules.
- Exact bounded work budgets and scheduling/fairness between observations, results, requests, controls, and physics catch-up.
- Retry/timeouts, liveness detection, and result transmission order; duplicate retention alone does not specify retries.
- Request eligibility during catch-up, same-boundary ordering, and handling of unresolved operations at experiment termination.
- Readiness API/library, detailed socket integration, shutdown handling, and storage representation; the simulator's single event loop and nonblocking TCP policy are accepted.

These must be resolved at the relevant implementation stage. The architecture decision does not imply that the wire contract is already complete.

## Relationship to other documents

This record resolves observation buffering, reconnect identity, and terminal result meanings left open in [decision 003](003-simulation-and-flight-timing.md). Its selected actuator status meanings apply to this simulation–flight interface. The flight-to-ground command lifecycle remains a separate contract; its use of accepted versus completed must not be silently conflated with this interface.

No other specification or decision record is changed by creating this document. Reconciliation edits require the developer's explicit request.

## Revisit triggers

Revisit if measured traffic/fairness needs justify separate connections, the single event loop cannot meet selected timing targets with the required workload, a second controller is required, historical-result handling becomes too complex, or deployment moves beyond the local Mac.
