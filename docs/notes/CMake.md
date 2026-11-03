# CMake


## Simulation

To compile for the models: `cmake --build build/debug --target tiny_sat_sim_models`

To build the tests: `cmake --build build/debug --target tiny_sat_sim_tests`

To run the tests: `ctest --test-dir build/debug --output-on-failure --no-tests=error`
