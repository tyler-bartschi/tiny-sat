# 006 — C++ simulation and a shared CMake build

**Status:** accepted, November 1, 2026. C++20, Apple Clang, and Catch2 are selected below. Minimum tool/dependency versions and detailed build configuration remain open.

## Context

Flight software is planned in C++. Simulation runs separately, owns physical state and simulation time, and communicates with flight through TCP. The developer has selected one repository and wants to write the implementation.

This record captures the C++ simulation, shared CMake build, C++20 baseline, initial Apple Clang toolchain, and Catch2 testing choices.

## Accepted decisions

### 1. Implement simulation in C++

Both simulation and flight are C++ applications. They remain separate executables and processes with their existing observation, actuator, timing, and state-ownership boundaries.

A common language allows consistent compiler and test tooling and leaves selected implementation reuse possible. It does not grant flight direct access to simulation truth or require the two components to share a protocol implementation.

Ground-service and UI implementation languages remain separate decisions. Choosing C++ for these two components does not select C++ for the entire repository.

### 2. Use one shared build with separate targets

Provide one build entry point for the C++ project, with separate simulation and flight executable targets. Support building the selected component and its actual dependencies without requiring the unrelated executable to be rebuilt.

Coordinate common settings through the shared build while keeping source ownership and component-specific dependencies explicit. A shared build does not merge runtime state, require one large configuration file, or create a dependency between the two applications merely because they communicate.

Tests and genuinely reusable libraries can become additional targets when needed. No shared library, class structure, or protocol-code reuse is selected by this decision. Detailed target names and configuration-file organization remain open.

### 3. Use CMake

Use CMake to define the C++ targets, their build requirements, and dependencies. CMake is the build-configuration tool; the compiler is selected separately in decision 5 below. The underlying build backend/generator and editor remain open.

The initial build supports development on the Mac. Raspberry Pi deployment and cross-compilation configuration are deferred until that work is explicitly undertaken.

### 4. Use C++20 for simulation and flight

Use C++20 as the common language-standard baseline for both applications and their project-owned C++ test code. Express the requirement explicitly in the CMake configuration rather than relying on compiler defaults or silently accepting an older language mode.

This decision selects the language baseline, not a requirement to use every C++20 facility. Availability of particular compiler and standard-library features must be checked when introduced. C++23-specific requirements are outside the selected baseline unless the developer later revises it.

Compiler-extension policy and any narrower feature restrictions remain open.

### 5. Initially use Apple Clang on the Mac

Use the existing Apple Clang toolchain for initial simulation and flight development. The inspected installation is provided by the Mac's Command Line Tools. A separately installed LLVM Clang or GCC is not selected for the initial build.

Apple Clang's compiler version and the C++20 language standard are independent version choices. The observed Apple Clang 21.0.0 is environment information, not an exact required or minimum version. Record the actual selected compiler in build/verification evidence and verify the features the project uses.

Future toolchain changes or Raspberry Pi support require a separate compatibility/build decision.

### 6. Use Catch2 for C++ tests

Use Catch2, with Homebrew as the initial local installation route, following the developer's existing experience. Catch2 provides documented CMake integration and test-discovery support: [Catch2 CMake integration](https://catch2-temp.readthedocs.io/en/latest/cmake-integration.html). Homebrew provides the [Catch2 package](https://formulae.brew.sh/formula/catch2).

Keep Catch2 as a test dependency rather than a runtime dependency of the flight or simulation executables. Its exact major/minor version, supported range, registration details, and any reproducible CI/dependency-pinning policy remain open. Do not assume an installed package's version or compatibility without checking it during setup.

This decision does not install Catch2 or create tests. A broader dependency-acquisition policy is not selected merely by choosing this initial local route.

## Alternatives and tradeoffs

- A Python simulator could support a distinct modeling environment but would add another runtime/tooling setup and cross-language protocol implementation. C++ was selected to concentrate the initial work in a common language.
- Independent component builds could provide self-contained configuration but would duplicate some settings and require additional integration coordination. A shared build was selected while preserving separate targets.
- Meson was considered as another target-oriented build tool. CMake was selected; maintainable conventions and target-specific requirements will still need deliberate design.
- C++23 was considered; C++20 was selected as the initial modern baseline. Use the existing Apple Clang installation rather than adding a separately managed compiler initially.
- Catch2 was selected to retain a familiar testing workflow and use its CMake integration. Homebrew is convenient locally; version reproducibility still needs an explicit policy.

## Verification intent

When implementation begins, verify that one documented CMake setup can configure the C++ project, build simulation and flight separately, and build both together. Verify that each executable runs as its own process and that component dependencies match their actual requirements.

Use a clean build directory to verify reproducibility, identify Apple Clang as the configured compiler, and verify explicit C++20 configuration. Add Catch2 test targets as tests acquire real responsibilities and verify package discovery and linking with the selected local version. Build files, source files, tests, and launch commands are not created by this decision record.

Read-only environment inspection on November 1, 2026 found Apple Clang 21.0.0 and CMake 4.3.3 on the Mac. These observations are not selected minimum versions and do not establish support for every feature of a C++ standard. No compilation or compatibility probe was performed.

## Remaining decisions

- Detailed C++20 feature/extension policy; the language-standard baseline is accepted.
- Minimum supported Apple Clang version and specific compiler/library feature checks; the initial compiler family is accepted.
- CMake minimum version, generator/backend, presets, build-directory conventions, and actual target names/layout.
- Warning/diagnostic policy, debug/release configuration, and available runtime diagnostics.
- Catch2 version/range, test registration, and broader dependency acquisition/versioning or CI policy; Catch2 and its initial local Homebrew route are selected.
- Whether selected utilities or serialization code should be shared between components.
- Any later integration of non-C++ ground tooling with a top-level developer workflow.

## Revisit triggers

Revisit if components acquire independent release or toolchain needs, a concrete modeling requirement justifies a different simulator language, or deployment beyond the Mac requires different build coordination.
