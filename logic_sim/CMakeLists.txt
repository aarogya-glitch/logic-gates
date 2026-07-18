cmake_minimum_required(VERSION 3.10)
project(DigitalLogicSimulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(include)

# --- core logic library (no GUI dependency) --------------------------------
add_library(logic_core STATIC
    src/Gate.cpp
    src/Gates.cpp
    src/Circuit.cpp
    src/TruthTable.cpp
    src/FileManager.cpp
)

# --- console test / demo (always builds, doesn't need SFML) ---------------
add_executable(test_core src/test_core.cpp)
target_link_libraries(test_core logic_core)

# --- GUI application (requires SFML) ---------------------------------------
find_package(SFML 2.5 COMPONENTS graphics window system)
if(SFML_FOUND)
    add_executable(logic_sim_gui src/main.cpp)
    target_link_libraries(logic_sim_gui logic_core sfml-graphics sfml-window sfml-system)
else()
    message(WARNING "SFML not found - 'logic_sim_gui' target skipped. "
                     "Install it with: sudo apt install libsfml-dev")
endif()
