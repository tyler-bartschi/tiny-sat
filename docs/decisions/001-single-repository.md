# 001 — Keep the project in one repository

**Status:** accepted, October 31, 2026.

## Context and decision

Keep flight software, spacecraft simulation, ground control, protocol contracts, scenarios, verification, and documentation in one repository. The developer selected this strategy during Phase 0.

These components form one experiment and will evolve together. One repository lets a change update both endpoints of an interface, its documentation, and verification together. Separate repositories could provide independent ownership and releases, but would add cross-repository coordination at this stage.

## Consequences

One repository may contain multiple executables, languages, and build tools. Folder layout, build organization, and runtime boundaries remain separate decisions. Components should have identifiable responsibilities and should eventually be buildable/testable without unrelated dependencies where practical.

Protocol integration and the clean-setup verification scenario should operate from one checkout. This decision does not select a toolchain or prescribe shared implementation code between components.

## Initial environment

All components run on the developer's Mac initially. A future Raspberry Pi flight deployment remains an option, rather than an initial implementation or verification requirement.

Keep flight behavior, simulated hardware, and ground control conceptually distinct. Moving flight software to another computer may require a sensor/action transport as well as different build and deployment procedures; this decision does not require implementing those now.

## Revisit trigger

Reconsider repository boundaries if components acquire independent users, maintainers, or release needs. Internal directory layout remains open; the Phase 0 specification's tree is a proposal.
