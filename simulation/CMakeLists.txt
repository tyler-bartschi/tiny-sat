add_library(tiny_sat_sim_models STATIC
        src/power_model.cpp
        include/tiny-sat/simulation/power_model.hpp
        include/tiny-sat/simulation/time.hpp
)

target_include_directories(tiny_sat_sim_models
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(tiny_sat_sim_models
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif ()