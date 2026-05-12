cmake_minimum_required(VERSION 3.16)
project(sysmon_metrics_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O0 -g")

include_directories(
    ${CMAKE_SOURCE_DIR}/MetricCollector/include
    ${CMAKE_SOURCE_DIR}/MetricConverter/include
    ${CMAKE_SOURCE_DIR}/nlohmann
    )
set(PATH_TO_METRIC_COLLECTOR "${CMAKE_SOURCE_DIR}/MetricCollector")
set(PATH_TO_METRIC_CONVERTER "${CMAKE_SOURCE_DIR}/MetricConverter")
set(PATH_TO_NLOHMANN "${CMAKE_SOURCE_DIR}/nlohmann")

include_directories(
    ${PATH_TO_METRIC_COLLECTOR}/include
    ${PATH_TO_METRIC_CONVERTER}/include
    ${PATH_TO_NLOHMANN}          
)

add_executable(test_metrics
    ${PATH_TO_METRIC_COLLECTOR}/src/main.cpp
    ${PATH_TO_METRIC_COLLECTOR}/src/ProcReader.cpp
    ${PATH_TO_METRIC_COLLECTOR}/src/MetricsCollector.cpp
    ${PATH_TO_METRIC_COLLECTOR}/src/CpuUsageCalculator.cpp
    ${PATH_TO_METRIC_COLLECTOR}/src/ProcessCollector.cpp
    ${PATH_TO_METRIC_CONVERTER}/src/FrontendTableBuilder.cpp
    ${PATH_TO_METRIC_CONVERTER}/src/MetricsSorter.cpp
)
target_link_libraries(test_metrics pthread)