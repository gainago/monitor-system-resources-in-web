#pragma once

#include "SystemMetrics.h"
#include "ProcessInfo.h"
#include <vector>

namespace sysmon {

/**
 * @brief Полный снимок системы: общие метрики и список процессов.
 */
struct FullSystemSnapshot {
    SystemMetrics system;
    std::vector<ProcessInfo> processes;
};

} // namespace sysmon