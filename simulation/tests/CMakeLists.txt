find_package(Catch2 3 CONFIG REQUIRED)

add_executable(tiny_sat_sim_tests
    power_model_tests.cpp
)

target_link_libraries(tiny_sat_sim_tests
    PRIVATE
        tiny_sat_sim_models
        Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(tiny_sat_sim_tests)