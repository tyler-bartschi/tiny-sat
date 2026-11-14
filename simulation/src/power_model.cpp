#include <cmath>
#include <stdexcept>
#include <string>
#include <tiny-sat/simulation/power_model.hpp>
#include <tiny-sat/simulation/time.hpp>

namespace tiny_sat::simulation {
PowerModel::PowerModel(const PowerConfig& config)
    : battery_capacity_wh_(config.battery_capacity_wh), baseline_draw_w_(config.baseline_draw_w) {
    check_pos_and_finite(config.battery_capacity_wh, "battery_capacity_wh");
    check_pos_and_finite(config.baseline_draw_w, "baseline_draw_w");
}

[[nodiscard]] PowerStepResult PowerModel::execute_interval(
    const PowerState& initial_state, const PowerInputs& inputs,
    const SimulationInterval& interval) const {
    // Input validations
    validate_state(initial_state, " (initial_state)");
    validate_inputs(inputs);
    if (interval < SimulationInterval::zero() || !std::isfinite(interval.count())) {
        throw std::invalid_argument("interval must be a finite, positive number or zero");
    }

    PowerReport power_report{
        .curtailed_energy_wh = 0.0,
        .unmet_energy_wh = 0.0,
        .supplied_energy_wh = 0.0,
        .battery_energy_delta_wh = 0.0,
    };

    if (interval == SimulationInterval::zero()) {
        // if interval is 0, return initial_state and a report of all 0s
        return PowerStepResult{
            .ending_state = initial_state,
            .report = power_report,
        };
    }

    PowerState ending_state{
        .battery_health = BatteryHealth::Optimal,
        .stored_energy_wh = 0.0,
    };

    // compute the delta, total power draw in wh over the interval, and the raw ending energy (can
    // be negative or > capacity)
    const double delta = interval.count() / 3600.0;

    const double total_draw_w = baseline_draw_w_ + inputs.power_draw_w;
    check_finite_for_overflow(
        total_draw_w,
        "execute_interval: baseline_draw_w + inputs.power_draw_w results in an overflow");

    const double power_draw_tot_wh = total_draw_w * delta;
    check_finite_for_overflow(power_draw_tot_wh,
                              "execute_interval: total_draw_wh * delta results in an overflow");

    const double generated_energy_wh = inputs.available_generation_w * delta;
    check_finite_for_overflow(
        generated_energy_wh,
        "execute_interval: inputs.available_generation_w * delta results in an overflow");

    const double raw_ending_energy =
        initial_state.stored_energy_wh + (generated_energy_wh - power_draw_tot_wh);
    check_finite_for_overflow(raw_ending_energy,
                              "execute_interval: initial_state.stored_energy_wh + "
                              "(generated_energy_wh - power_draw_tot_wh) results in an overflow");

    if (raw_ending_energy >= 0 && raw_ending_energy <= battery_capacity_wh_) {
        // if raw_ending_energy is between 0 and the capacity inclusive, all energy was supplied and
        // the battery changed
        ending_state.stored_energy_wh = raw_ending_energy;
        power_report.supplied_energy_wh = power_draw_tot_wh;
        power_report.battery_energy_delta_wh = raw_ending_energy - initial_state.stored_energy_wh;
    } else if (raw_ending_energy < 0) {
        // raw_ending_energy is negative, meaning generation and the battery could not supply all
        // power needed
        ending_state.stored_energy_wh = 0.0;
        power_report.unmet_energy_wh = -1 * raw_ending_energy;
        power_report.supplied_energy_wh = power_draw_tot_wh - power_report.unmet_energy_wh;
        power_report.battery_energy_delta_wh = -1 * initial_state.stored_energy_wh;
    } else if (raw_ending_energy > battery_capacity_wh_) {
        // raw_ending_energy is positive, meaning generation exceeded both the power needed and
        // remaining change in the battery
        ending_state.stored_energy_wh = battery_capacity_wh_;
        power_report.curtailed_energy_wh = raw_ending_energy - battery_capacity_wh_;
        power_report.supplied_energy_wh = power_draw_tot_wh;
        power_report.battery_energy_delta_wh =
            battery_capacity_wh_ - initial_state.stored_energy_wh;
    }

    validate_state(ending_state, " (ending_state)");
    validate_report(power_report);

    return PowerStepResult{
        .ending_state = ending_state,
        .report = power_report,
    };
}

void PowerModel::validate_state(const PowerState& state, const std::string& name_append) const {
    check_pos_and_finite(state.stored_energy_wh, "stored_energy_wh" + name_append);
    if (state.stored_energy_wh > battery_capacity_wh_) {
        throw std::invalid_argument(
            "stored_energy_wh (initial_state) must be less than battery_capacity_wh");
    }
}

void PowerModel::validate_inputs(const PowerInputs& inputs) {
    check_pos_and_finite(inputs.available_generation_w, "available_generation_w (inputs)");
    check_pos_and_finite(inputs.power_draw_w, "power_draw_w (inputs)");
}

void PowerModel::validate_report(const PowerReport& report) {
    check_pos_and_finite(report.curtailed_energy_wh, "curtailed_energy_wh (power_report)");
    check_pos_and_finite(report.unmet_energy_wh, "unmet_energy_wh (power_report)");
    check_pos_and_finite(report.supplied_energy_wh, "supplied_energy_wh (power_report)");
    if (!std::isfinite(report.battery_energy_delta_wh)) {
        throw std::invalid_argument(
            "battery_power_delta_wh (power_report) must be a finite number (pos or neg)");
    }
}

void PowerModel::check_pos_and_finite(const double& val, const std::string& name) {
    if (val < 0 || !std::isfinite(val)) {
        throw std::invalid_argument(name + "must be a finite, positive number or zero");
    }
}

void PowerModel::check_finite_for_overflow(const double& val, const std::string& message) {
    if (!std::isfinite(val)) {
        throw std::overflow_error(message);
    }
}
}  // namespace tiny_sat::simulation
