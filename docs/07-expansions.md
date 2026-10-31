# 07 — Expansion paths after the core release

## How to select an extension

Add one new source of complexity at a time. Name its learning objective, the existing boundary it touches, its new observable behavior, and its verification scenario before implementation. Keep the core mission scenario as a regression check.

Do not reserve an abstraction for every hypothetical future device. Preserve the small contracts that already have a purpose: observations into flight, actions into the plant, messages across transport, and state/results into ground.

## More capable spacecraft models

| Extension | New behavior and learning | Prerequisite and acceptance idea |
| --- | --- | --- |
| Sunlight/contact schedule | Repeating eclipse and contact windows drive power and communication availability | Stable power model; verify expected depletion/charging and contact boundaries |
| Payload mission activity | A timed science/data-generation activity consumes power and storage | Command lifecycle and payload model; interrupt in SAFE and explain incomplete work |
| Attitude dynamics | Orientation and angular rate evolve under commanded torque | Coordinate/unit decision, stable integrator; verify simple known rotations before control |
| Attitude control | Controller commands actuators using sensor estimates | Attitude plant, sensors, saturation; test disturbances and actuator limits |
| Richer sensors | Noise, bias, drift, stuck values, redundant channels | Existing truth/observation separation; specify which failures are detectable and why |
| Richer thermal/power models | Multiple thermal nodes, variable radio load, battery efficiency or degradation | Validated simple model; compare against independent reference cases |
| Actual power loss | Bus depletion stops/reboots computation with defined persistence | Boot/identity contract; verify interrupted commands remain uncertain and startup is safe |
| Watchdog behavior | A separate supervisor detects a stalled flight executive | Independent liveness observation; stall the actual monitored execution path |

Do not begin attitude work by adding visually plausible numbers to an operational field without a coordinate convention. Define axes, reference frame, angle units, timestamp, and quaternion ordering if applicable.

## Richer operations and ground tools

**Telemetry replay:** replay saved data into a clearly marked historical view. Separate replay from live command authority. Verify that replay cannot accidentally submit commands to the spacecraft.

**Contact planning:** predict windows, queue intended activities, and reconcile them with actual contact. This introduces deliberate store-and-forward commanding; it needs a new expiry/authorization policy rather than removal of the baseline stale-command safeguard.

**Command sequences:** execute a bounded series with preconditions, cancellation, and failure policy. Specify which side owns progress and what survives disconnection. Test interruption halfway through, not just successful completion.

**Multiple operators:** add explicit ownership/authorization and an auditable command source. A shared dashboard with multiple writable clients is not automatically safe multi-operator control.

**Multiple spacecraft:** introduce routing, independent time/boot/sequence domains, separate current views, and resource fairness. Verify that a command for one spacecraft cannot be applied to another.

**External telemetry consumers:** expose a documented read-only API or export adapter. Keep field units, quality, schema version, and source/receipt times intact. Verify an independently written consumer can interpret fixtures.

## Raspberry Pi or another embedded computer

Keep the plant simulated on the host initially and move only the flight application to the target. This requires an explicit observation/action transport and introduces latency/failure at a boundary that was previously in-process.

Your learning goals would be target builds, platform dependencies, deployment, process supervision, clock behavior, and measurement under a different scheduler. First reproduce the core scenarios. Then compare timing and resource data with the Mac results under equivalent workload.

Choose whether observations are synchronous, buffered, or timestamped asynchronous messages, and define what happens when the plant connection is lost. Do not hide a new sensor-transport failure behind “healthy” cached values. A Pi application on a general-purpose OS is not automatically a hard real-time system.

## Real devices and hardware-in-the-loop

A first hardware extension could replace one simulated sensor with a real low-voltage temperature sensor while the rest of the spacecraft remains simulated. Preserve the same observation quality/time contract, add device errors, and keep known model truth separate from physical measurement.

Hardware-in-the-loop adds physical limits and failure consequences. Define the device boundary and permitted actions before controlling actuators. Reusing a simulated payload-enable command is not sufficient preparation for arbitrary real hardware control.

## Protocol and ecosystem integration

After stabilizing your custom protocol, consider adapting to an established spacecraft packet convention or comparing your approach with a framework such as F Prime. Start with one telemetry stream or a read-only bridge. State precisely which portion is compatible and test against an independent consumer; do not claim standards compliance from a similar header.

Keep the flight domain independent enough that a new wire adapter does not require rewriting fault policy. Conversely, do not build a general protocol abstraction before you have a second real use case.

## Network exposure and deployment

Moving beyond loopback changes the trust boundary. Add a concrete decision covering authentication, authorization, transport protection, command replay, API access, configuration/secrets, and logging. CRCs and sequence numbers do not provide those protections.

A read-only public portfolio demo is a different deployment from a remotely commandable flight service. Consider a prerecorded or isolated simulation view first. Specify the intended exposure rather than inheriting local-development assumptions.

## Higher reliability and execution realism

Durable command outcomes across a flight crash require persistence and recovery semantics around the effect itself. Define the crash points, whether actions can be repeated safely, and what reconciliation can establish. This is substantially more demanding than retaining a command ID in a memory cache.

Other useful extensions include persistent event downlink, limited bandwidth and prioritization, disk-full behavior, longer endurance runs, deterministic integration replay, and comparison with a real-time execution environment. Each requires new bounds and scenarios, not just an additional configuration toggle.

## A useful backlog entry

For an idea you defer, record its name, learning objective, prerequisites, changed contracts, one demonstration, and why it is outside the current release. Prioritize the idea that adds the most useful experience per new dependency. New models, new hardware, and new deployment infrastructure should rarely arrive in the same increment.
