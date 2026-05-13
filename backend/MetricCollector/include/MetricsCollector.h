#pragma once

#include "IProcReader.h"
#include "SystemMetrics.h"
#include "ProcessInfo.h"
#include "ProcessCollector.h"
#include "FullSystemSnapshot.h"
#include <memory>
#include <chrono>
#include <mutex>

namespace sysmon {

class MetricsCollector {
public:
    MetricsCollector(std::unique_ptr<IProcReader> reader, unsigned int cpu_core_count);

    /**
     * @brief Собрать все метрики системы и список процессов.
     * Блокируется на ~1 секунду для получения дельты CPU.
     * @return FullSystemSnapshot.
     */
    FullSystemSnapshot collect();

private:
    std::mutex mutex_;
    std::shared_ptr<IProcReader> reader_; // shared, т.к. ProcessCollector тоже его использует
    unsigned int cpu_core_count_;
    std::unique_ptr<ProcessCollector> process_collector_;
    // поля для системного CPU
    std::vector<unsigned long long> prev_cpu_ticks_;
    bool first_call_ = true;
    std::chrono::steady_clock::time_point last_collect_time_;
};

} // namespace sysmon