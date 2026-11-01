# 002 - Repository Structure

**Status:** Accepted, October 31, 2026.

## Context and Decision

The following general repository structure will be used to organize systems and modules by responsilibity. 

```text
tiny-sat/
  ├─config/
  ├─docs/
  │   ├─decisions/
  ├─evidence/
  ├─flight/
  ├─ground/
  │   ├─service/
  │   ├─ui/
  ├─protocol/
  ├─scenarios/
  ├─tests/
  └─tools/
```

`config` - Configurations for running the application  
`docs` and `docs/decisions` - architectural decisions and documentation for the project  
`evidence` - Result summaries, plots, references, etc to demonstrate the project working  
`flight` - Main flight control computer, controlling modes, scheduling, responding to commands, reading sensor data, and broadcasting telemetry  
`ground` - Ground control. `ground/service` controls communicating with the spacecraft and providing the API for the browser, `ground/ui` contains the UI for the browser to display information  
`protocol` - shared communication contract between flight and ground  
`scenarios` - experiment defintions, inlcuding starting conditions, changes, faults, etc  
`tests` - tests that test multiple components (like integration tests). Unit tests should live next to tested component  
`tools` - development tools for aiding various operations (for the developer)

## Consequences

Defines the state of the project moving forward. Interface boundaries should be well-known and monitored, and individual folders should maintain their contracts.

## Revist Trigger

Revisit if implementation requires different structure.
