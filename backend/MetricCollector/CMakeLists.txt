cmake_minimum_required(VERSION 3.16)
project(sysmon_metrics_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(test_metrics
    src/main.cpp
    src/ProcReader.cpp
    src/MetricsCollector.cpp
    src/CpuUsageCalculator.cpp
    src/ProcessCollector.cpp
)

target_link_libraries(test_metrics pthread)