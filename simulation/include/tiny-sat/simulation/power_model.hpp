#pragma once

#include <string>
#include <tiny-sat/simulation/time.hpp>

namespace tiny_sat::simulation {

enum class BatteryHealth {
    Optimal,
};

// Initial configuration for the power model, values treated as consistent between calculation calls
struct PowerConfig {
    // battery_capacity in Wh
    double battery_capacity_wh;
    // baseline_draw in W
    double baseline_draw_w;
};

// State of the battery, snapshot in time (time intentionally not exposed to the power model)
struct PowerState {
    // status of the battery, may be expanded/useful later
    BatteryHealth battery_health;
    // current amount of stored energy in Wh
    double stored_energy_wh;
};

// Inputs to the model for an interval calculation
struct PowerInputs {
    // amount of power generation in W
    double available_generation_w;
    // amount of additional power draw in W, in the future perhaps make this a more nuanced value(s)
    double power_draw_w;
};

// Report provided by the model at the end of an interval
struct PowerReport {
    // amount of energy curtailed (unused) in Wh
    double curtailed_energy_wh;
    // amount of unmet energy demand in Wh
    double unmet_energy_wh;
    // amount of energy actually supplied to loads in Wh
    double supplied_energy_wh;
    // amount of energy change in the battery, can be negative, in Wh
    double battery_energy_delta_wh;
};

// Result provided back to runner, including both the PowerState and PowerReport at the end of an
// interval
struct PowerStepResult {
    // state of the battery at the end of the interval
    PowerState ending_state;
    // report provided by the battery at the end of the interval
    PowerReport report;
};

/*
 * The PowerModel, initialized with initial configuration state and can execute an interval provided
 * a starting state and an interval time (seconds)
 * Behavior of PowerModel:
 *
 * Invalid initial configuration (negative/non-finite battery_capacity, negative/non-finite
 * baseline_draw): throws error (std::invalid_argument)
 * Invalid initial_state provided to execute_interval (negative/non-finite/above capacity
 * stored_energy_wh): throws error (std::invalid_argument) Invalid inputs provided to
 * execute_interval (negative/non-finite available_generation_w, negative/non-finite power_draw_w):
 * throws error (std::invalid_argument) Invalid interval provided to execute_interval
 * (negative/non-finite): throws error (std::invalid_argument)
 * For a positive interval, execute_interval throws std::overflow_error if total draw in W,
 * requested energy, generated energy, or unclamped ending energy is non-finite after calculation.
 *
 * Configuration accepts battery_capacity_wh = 0 (no battery) and baseline_draw_w = 0 (no baseline
 * draw)
 * execute_interval initial_state accepts stored_energy_wh = 0 (no stored energy, battery empty)
 * execute_interval inputs accepts available_generation_w = 0 (no generation, all power must
 * come from battery), power_draw_w = 0 (no additional energy required)
 * execute_interval accepts interval = 0 (no time passes, returns initial state with no change,
 * report is all 0s)
 *
 */
class PowerModel {
public:
    explicit PowerModel(const PowerConfig& config);

    /* inputs provides the inputs to the model
     * the interval provides the interval time in seconds, as defined by SimulationInterval
     */
    [[nodiscard]] PowerStepResult execute_interval(const PowerState& initial_state,
                                                   const PowerInputs& inputs,
                                                   const SimulationInterval& interval) const;

private:
    const double battery_capacity_wh_;
    const double baseline_draw_w_;

    void validate_state(const PowerState& state, const std::string& name_append) const;
    static void validate_inputs(const PowerInputs& inputs);
    static void validate_report(const PowerReport& report);
    static void check_pos_and_finite(const double& val, const std::string& name);
    static void check_finite_for_overflow(const double& val, const std::string& message);
};

}  // namespace tiny_sat::simulation
