#pragma once

#include "IProcReader.h"
#include "ProcessInfo.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace sysmon {

/**
 * @brief Сборщик информации о процессах, с кэшированием для расчёта CPU%.
 *
 * Использует IProcReader для чтения /proc.
 * При каждом вызове collectProcesses() обновляет список процессов,
 * вычисляет cpu_percent и mem_percent для каждого.
 */
class ProcessCollector {
public:
    explicit ProcessCollector(std::shared_ptr<IProcReader> reader, unsigned int num_cpus);

    /**
     * @brief Собрать список процессов с актуальными метриками.
     * @param mem_total_kb общий объём памяти для расчёта mem%.
     * @return вектор ProcessInfo.
     */
    std::vector<ProcessInfo> collectProcesses(unsigned long long mem_total_kb);

private:
    struct ProcessSnapshot {
        unsigned long long process_ticks; // utime+stime
        unsigned long long total_ticks;   // общие тики CPU на момент замера
    };

    std::shared_ptr<IProcReader> reader_;
    unsigned int num_cpus_;
    std::unordered_map<int, ProcessSnapshot> prev_snapshots_;
};

} // namespace sysmon