#pragma once

#include <chrono>

namespace tiny_sat::simulation {
// SimulationInterval represents a duration in seconds for the simulation
using SimulationInterval = std::chrono::duration<double>;
}  // namespace tiny_sat::simulation
