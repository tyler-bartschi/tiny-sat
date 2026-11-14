#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <limits>
#include <stdexcept>
#include <tiny-sat/simulation/power_model.hpp>
using tiny_sat::simulation::SimulationInterval;

namespace tss = tiny_sat::simulation;

namespace {
constexpr double nan_val = std::numeric_limits<double>::quiet_NaN();
constexpr double pos_inf = std::numeric_limits<double>::infinity();
constexpr double neg_inf = -std::numeric_limits<double>::infinity();
constexpr double limit_vals[] = {nan_val, pos_inf, neg_inf};

constexpr auto BASE_INTERVAL = SimulationInterval{3600};
constexpr double EPSILON = 1e-12;

struct Row {
    const char* name;
    double start_wh;
    double available_generation_w;
    double power_draw_w;
    double end_wh;
    double curtailed_wh;
    double unmet_wh;
    double supplied_wh;
    double battery_delta_wh;
};

tss::PowerModel create_power_model(const double& bat_cap_wh, const double& base_draw_w) {
    const tss::PowerConfig config{
        .battery_capacity_wh = bat_cap_wh,
        .baseline_draw_w = base_draw_w,
    };

    return tss::PowerModel(config);
}

tss::PowerState create_power_state(const double& stored_energy_wh) {
    return tss::PowerState{
        .battery_health = tss::BatteryHealth::Optimal,
        .stored_energy_wh = stored_energy_wh,
    };
}

tss::PowerInputs create_power_inputs(const double& avail_gen_w, const double& power_draw_w) {
    return tss::PowerInputs{
        .available_generation_w = avail_gen_w,
        .power_draw_w = power_draw_w,
    };
}

void check_state(const tss::PowerState& expected, const tss::PowerState& actual) {
    REQUIRE(expected.stored_energy_wh == actual.stored_energy_wh);
    REQUIRE(expected.battery_health == actual.battery_health);
}

void check_report(const tss::PowerReport& expected, const tss::PowerReport& actual) {
    REQUIRE(expected.curtailed_energy_wh == actual.curtailed_energy_wh);
    REQUIRE(expected.supplied_energy_wh == actual.supplied_energy_wh);
    REQUIRE(expected.unmet_energy_wh == actual.unmet_energy_wh);
    REQUIRE(expected.battery_energy_delta_wh == actual.battery_energy_delta_wh);
}

const tss::PowerModel base_power_model = create_power_model(10, 4);

void perform_checks(const tss::PowerStepResult& result, const Row* row) {
    CHECK_THAT(result.ending_state.stored_energy_wh,
               Catch::Matchers::WithinAbs(row->end_wh, EPSILON));
    CHECK_THAT(result.report.curtailed_energy_wh,
               Catch::Matchers::WithinAbs(row->curtailed_wh, EPSILON));
    CHECK_THAT(result.report.unmet_energy_wh, Catch::Matchers::WithinAbs(row->unmet_wh, EPSILON));
    CHECK_THAT(result.report.supplied_energy_wh,
               Catch::Matchers::WithinAbs(row->supplied_wh, EPSILON));
    CHECK_THAT(result.report.battery_energy_delta_wh,
               Catch::Matchers::WithinAbs(row->battery_delta_wh, EPSILON));
}

void check_rows(const Row* rows, const int& row_len) {
    for (int i = 0; i < row_len; i++) {
        const Row* row = rows + i;

        INFO(row->name);
        const tss::PowerStepResult result = base_power_model.execute_interval(
            create_power_state(row->start_wh),
            create_power_inputs(row->available_generation_w, row->power_draw_w), BASE_INTERVAL);

        perform_checks(result, row);
    }
}

void check_rows_config(const double& capacity, const double& baseline,
                       const SimulationInterval& interval, const Row* rows, const int& row_len) {
    const auto power_model = create_power_model(capacity, baseline);
    for (int i = 0; i < row_len; i++) {
        const Row* row = rows + i;

        INFO(row->name);
        const tss::PowerStepResult result = power_model.execute_interval(
            create_power_state(row->start_wh),
            create_power_inputs(row->available_generation_w, row->power_draw_w), interval);

        perform_checks(result, row);
    }
}
}  // namespace

TEST_CASE("PowerModel invalid initial configuration throws", "[PowerModel][exceptions]") {
    tss::PowerConfig config{
        .battery_capacity_wh = -19.2,
        .baseline_draw_w = 4,
    };

    // negative battery_capacity_wh
    REQUIRE_THROWS_AS(tss::PowerModel(config), std::invalid_argument);

    for (const double& val : limit_vals) {
        // battery_capacity_wh as nan, pos inf, and neg inf
        config.battery_capacity_wh = val;
        REQUIRE_THROWS_AS(tss::PowerModel(config), std::invalid_argument);
    }

    config.battery_capacity_wh = 10;
    config.baseline_draw_w = -1.5;

    REQUIRE_THROWS_AS(tss::PowerModel(config), std::invalid_argument);

    for (const double& val : limit_vals) {
        config.baseline_draw_w = val;
        REQUIRE_THROWS_AS(tss::PowerModel(config), std::invalid_argument);
    }
}

TEST_CASE("PowerModel invalid initial_state throws", "[PowerModel][exceptions]") {
    const auto power_model = create_power_model(10, 10);

    tss::PowerState initial_state = create_power_state(-10);

    const tss::PowerInputs inputs = create_power_inputs(0, 0);

    REQUIRE_THROWS_AS(power_model.execute_interval(initial_state, inputs, SimulationInterval{10}),
                      std::invalid_argument);

    for (const double& val : limit_vals) {
        initial_state.stored_energy_wh = val;
        REQUIRE_THROWS_AS(
            power_model.execute_interval(initial_state, inputs, SimulationInterval{10}),
            std::invalid_argument);
    }
}

TEST_CASE("PowerModel invalid inputs throws", "[PowerModel][exceptions]") {
    const auto power_model = create_power_model(10, 10);

    const tss::PowerState initial_state = create_power_state(10);

    tss::PowerInputs inputs = create_power_inputs(-10, 4);

    REQUIRE_THROWS_AS(power_model.execute_interval(initial_state, inputs, SimulationInterval{10}),
                      std::invalid_argument);

    for (const double& val : limit_vals) {
        inputs.available_generation_w = val;
        REQUIRE_THROWS_AS(
            power_model.execute_interval(initial_state, inputs, SimulationInterval{10}),
            std::invalid_argument);
    }

    inputs.available_generation_w = 10;
    inputs.power_draw_w = -4;

    REQUIRE_THROWS_AS(power_model.execute_interval(initial_state, inputs, SimulationInterval{10}),
                      std::invalid_argument);

    for (const double& val : limit_vals) {
        inputs.power_draw_w = val;
        REQUIRE_THROWS_AS(
            power_model.execute_interval(initial_state, inputs, SimulationInterval{10}),
            std::invalid_argument);
    }
}

TEST_CASE("PowerModel invalid interval throws", "[PowerModel][exceptions]") {
    const auto power_model = create_power_model(10, 10);

    const tss::PowerState initial_state = create_power_state(10);

    const tss::PowerInputs inputs = create_power_inputs(10, 4);

    REQUIRE_THROWS_AS(power_model.execute_interval(initial_state, inputs, SimulationInterval{-10}),
                      std::invalid_argument);

    for (const double& val : limit_vals) {
        REQUIRE_THROWS_AS(
            power_model.execute_interval(initial_state, inputs, SimulationInterval{val}),
            std::invalid_argument);
    }
}

TEST_CASE("PowerModel with 0 interval returns initial state", "[PowerModel][boundary]") {
    const auto power_model = create_power_model(10, 10);

    const tss::PowerState initial_state = create_power_state(10);

    const tss::PowerInputs inputs = create_power_inputs(10, 4);

    const auto [ending_state, report] =
        power_model.execute_interval(initial_state, inputs, SimulationInterval{0});

    check_state(initial_state, ending_state);
    check_report(
        tss::PowerReport{
            .curtailed_energy_wh = 0,
            .unmet_energy_wh = 0,
            .supplied_energy_wh = 0,
            .battery_energy_delta_wh = 0,
        },
        report);
}

TEST_CASE("Core charging and discharging", "[PowerModel]") {
    constexpr Row rows[] = {
        {"Charges without reaching full", 3, 8, 2, 5, 0, 0, 6, 2},
        {"Discharges without reaching empty", 7, 4, 2, 5, 0, 0, 6, -2},
        {"Generation exactly matches demand", 5, 6, 2, 5, 0, 0, 6, 0},
    };

    check_rows(rows, std::size(rows));
}

TEST_CASE("Ending on empty battery", "[PowerModel][boundary]") {
    constexpr Row rows[] = {
        {"Ends just above empty", 2.25, 4, 2, 0.25, 0, 0, 6, -2},
        {"Reaches empty exactly at the end", 2, 4, 2, 0, 0, 0, 6, -2},
        {"Reaches empty before the end", 1.75, 4, 2, 0, 0, 0.25, 5.75, -1.75},
    };

    check_rows(rows, std::size(rows));
}

TEST_CASE("Ending on full battery", "[PowerModel][boundary]") {
    constexpr Row rows[] = {
        {"Ends just below full", 7.75, 8, 2, 9.75, 0, 0, 6, 2},
        {"Reaches full exactly at the end", 8, 8, 2, 10, 0, 0, 6, 2},
        {"Reaches full before the end", 8.25, 8, 2, 10, 0.25, 0, 6, 1.75},
    };

    check_rows(rows, std::size(rows));
}

TEST_CASE("Starts with an empty battery", "[PowerModel]") {
    constexpr Row rows[] = {
        {"Starts empty, no generation", 0, 0, 2, 0, 0, 6, 0, 0},
        {"Starts empty, generation covers part of demand", 0, 4, 2, 0, 0, 2, 4, 0},
        {"Starts empty, generation covers all demand", 0, 6, 2, 0, 0, 0, 6, 0},
        {"Starts empty, surplus begins charging", 0, 8, 2, 2, 0, 0, 6, 2},
    };

    check_rows(rows, std::size(rows));
}

TEST_CASE("Starts with a full battery", "[PowerModel]") {
    constexpr Row rows[] = {
        {"Starts full, discharges to cover a deficit", 10, 4, 2, 8, 0, 0, 6, -2},
        {"Starts full, generation exactly matches demand", 10, 6, 2, 10, 0, 0, 6, 0},
        {"Starts full, curtails all surplus", 10, 8, 2, 10, 2, 0, 6, 0},
    };

    check_rows(rows, std::size(rows));
}

TEST_CASE("No generation", "[PowerModel]") {
    constexpr Row rows[] = {
        {"No generation, battery covers all demand", 7, 0, 2, 1, 0, 0, 6, -6},
        {"No generation, battery covers part of demand", 3, 0, 2, 0, 0, 3, 3, -3},
    };

    check_rows(rows, std::size(rows));
}

TEST_CASE("Baseline load only", "[PowerModel]") {
    constexpr Row rows[] = {{"Baseline load only", 5, 4, 0, 5, 0, 0, 4, 0}};

    check_rows(rows, std::size(rows));
}

TEST_CASE("Extra power draw only", "[PowerModel]") {
    constexpr Row rows[] = {{"Extra power draw only", 5, 2, 4, 3, 0, 0, 4, -2}};

    check_rows_config(10, 0, BASE_INTERVAL, rows, std::size(rows));
}

TEST_CASE("No load and no generation", "[PowerModel]") {
    constexpr Row rows[] = {{"No load and no generation", 5, 0, 0, 5, 0, 0, 0, 0}};

    check_rows_config(10, 0, BASE_INTERVAL, rows, std::size(rows));
}

TEST_CASE("No load, some generation", "[PowerModel]") {
    constexpr Row rows[] = {
        {"No load, some generation fills battery with surplus", 8, 4, 0, 10, 2, 0, 0, 2}};

    check_rows_config(10, 0, BASE_INTERVAL, rows, std::size(rows));
}

TEST_CASE("No battery", "[PowerModel]") {
    constexpr Row rows[] = {
        {"No battery, generation below demand", 0, 4, 2, 0, 0, 2, 4, 0},
        {"No battery, generation matches demand", 0, 6, 2, 0, 0, 0, 6, 0},
        {"No battery, generation above demand", 0, 8, 2, 0, 2, 0, 6, 0},
    };

    check_rows_config(0, 4, BASE_INTERVAL, rows, std::size(rows));
}

TEST_CASE("Both configuration values zero", "[PowerModel]") {
    constexpr Row rows[] = {{"Both configuration values zero", 0, 0, 0, 0, 0, 0, 0, 0}};

    check_rows_config(0, 0, BASE_INTERVAL, rows, std::size(rows));
}

TEST_CASE("Zero time, despite a power surplus", "[PowerModel]") {
    constexpr Row rows[] = {{"Zero time, despite a power surplus", 5, 20, 2, 5, 0, 0, 0, 0}};

    check_rows_config(10, 4, SimulationInterval{0}, rows, std::size(rows));
}

TEST_CASE("Half hour discharge", "[PowerModel]") {
    constexpr Row rows[] = {{"Half hour discharge", 5, 3, 2, 3.5, 0, 0, 3, -1.5}};

    check_rows_config(10, 4, SimulationInterval{1800}, rows, std::size(rows));
}

TEST_CASE("Interval longer than an hour", "[PowerModel]") {
    constexpr Row rows[] = {{"Interval longer than an hour", 5, 4, 2, 2, 0, 0, 9, -3}};

    check_rows_config(10, 4, SimulationInterval{5400}, rows, std::size(rows));
}

TEST_CASE("10 ms charge", "[PowerModel]") {
    constexpr Row rows[] = {
        {"10 ms charge", 0.5, 10, 2, 45001 / 90000.0, 0, 0, 1 / 60000.0, 1 / 90000.0}};

    check_rows_config(10, 4, SimulationInterval{0.010}, rows, std::size(rows));
}

TEST_CASE("Successive Intervals", "[PowerModel]") {
    Row rows[] = {{"First half hour", 9, 10, 2, 10, 1, 0, 3, 1}};

    check_rows_config(10, 4, SimulationInterval{1800}, rows, std::size(rows));

    rows[0].name = "Second half-hour, starting from the first result";
    rows[0].start_wh = 10;
    rows[0].curtailed_wh = 2;
    rows[0].battery_delta_wh = 0;

    check_rows_config(10, 4, SimulationInterval{1800}, rows, std::size(rows));

    rows[0].name = "Same hour as one independent call";
    rows[0].start_wh = 9;
    rows[0].curtailed_wh = 3;
    rows[0].supplied_wh = 6;
    rows[0].battery_delta_wh = 1;

    check_rows_config(10, 4, SimulationInterval{3600}, rows, std::size(rows));

    rows[0].name = "Next half-hour after the split run, generation drops to zero";
    rows[0].start_wh = 10;
    rows[0].available_generation_w = 0;
    rows[0].end_wh = 7;
    rows[0].curtailed_wh = 0;
    rows[0].supplied_wh = 3;
    rows[0].battery_delta_wh = -3;

    check_rows_config(10, 4, SimulationInterval{1800}, rows, std::size(rows));
}

TEST_CASE("Starting above capacity", "[PowerModel][exceptions]") {
    REQUIRE_THROWS_AS(base_power_model.execute_interval(create_power_state(11),
                                                        create_power_inputs(2, 2), BASE_INTERVAL),
                      std::invalid_argument);
}

TEST_CASE("Individual values too large causing an overflow", "[PowerModel][exceptions]") {
    constexpr double max_finite = std::numeric_limits<double>::max();

    SECTION("Total power draw overflows when baseline and extra draw are added") {
        const auto model = create_power_model(10, max_finite);
        REQUIRE_THROWS_AS(model.execute_interval(create_power_state(5),
                                                 create_power_inputs(0, max_finite), BASE_INTERVAL),
                          std::overflow_error);
    }

    SECTION("Requested energy overflows when total draw is scaled by the interval") {
        const auto model = create_power_model(10, max_finite);
        REQUIRE_THROWS_AS(model.execute_interval(create_power_state(5), create_power_inputs(0, 0),
                                                 SimulationInterval{7200}),
                          std::overflow_error);
    }

    SECTION("Generated energy overflows when generation is scaled by the interval") {
        REQUIRE_THROWS_AS(base_power_model.execute_interval(create_power_state(5),
                                                            create_power_inputs(max_finite, 0),
                                                            SimulationInterval{7200}),
                          std::overflow_error);
    }

    SECTION("Unclamped ending energy overflows when surplus is added to stored energy") {
        const auto model = create_power_model(max_finite, 0);
        REQUIRE_THROWS_AS(model.execute_interval(create_power_state(max_finite),
                                                 create_power_inputs(max_finite, 0), BASE_INTERVAL),
                          std::overflow_error);
    }
}
