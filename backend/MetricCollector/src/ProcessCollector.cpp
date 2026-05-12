#include "ProcessCollector.h"
#include <algorithm>

namespace sysmon {

ProcessCollector::ProcessCollector(std::shared_ptr<IProcReader> reader, unsigned int num_cpus)
    : reader_(std::move(reader)), num_cpus_(num_cpus) {}

std::vector<ProcessInfo> ProcessCollector::collectProcesses(unsigned long long mem_total_kb) {
    auto pids = reader_->getPids();
    unsigned long long total_ticks = reader_->getTotalCpuTicks();
    std::vector<ProcessInfo> processes;
    processes.reserve(pids.size());

    for (int pid : pids) {
        auto info = reader_->readProcessInfo(pid);
        if (info.pid == 0) continue; // не удалось прочитать

        // Расчёт CPU%
        auto it = prev_snapshots_.find(pid);
        if (it != prev_snapshots_.end() && total_ticks > it->second.total_ticks) {
            unsigned long long delta_process = info.time_ticks - it->second.process_ticks;
            unsigned long long delta_total = total_ticks - it->second.total_ticks;
            if (delta_total > 0) {
                info.cpu_percent = 100.0 * delta_process / delta_total * num_cpus_;
            }
        }
        // Обновить кэш
        prev_snapshots_[pid] = {info.time_ticks, total_ticks};

        // MEM%
        if (mem_total_kb > 0) {
            info.mem_percent = 100.0 * info.res_kb / mem_total_kb;
        }

        processes.push_back(std::move(info));
    }

    // Удалить кэш для умерших процессов
    for (auto it = prev_snapshots_.begin(); it != prev_snapshots_.end(); ) {
        if (std::find(pids.begin(), pids.end(), it->first) == pids.end()) {
            it = prev_snapshots_.erase(it);
        } else {
            ++it;
        }
    }

    return processes;
}

} // namespace sysmon