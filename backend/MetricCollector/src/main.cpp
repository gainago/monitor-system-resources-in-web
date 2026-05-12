#include "MetricsCollector.h"
#include "ProcReader.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

int main() {
    using namespace sysmon;
    auto reader = std::make_unique<ProcReader>();
    unsigned int cores = reader->getCpuCount();
    std::cout << "Detected " << cores << " CPU cores\n";

    MetricsCollector collector(std::move(reader), cores);

    for (int i = 0; i < 5; ++i) {
        auto snapshot = collector.collect(); // ~1 сек
        std::cout << "\n=== Iteration " << i+1 << " ===\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "System CPU: " << snapshot.system.cpu_percent << "%, RAM: "
                  << snapshot.system.ram_percent << "%, Load: "
                  << snapshot.system.load_avg[0] << "\n";

        // Сортировка процессов по CPU%
        auto& procs = snapshot.processes;
        std::sort(procs.begin(), procs.end(),
                  [](const ProcessInfo& a, const ProcessInfo& b) {
                      return a.cpu_percent > b.cpu_percent;
                  });

        std::cout << "Top 5 processes by CPU:\n";
        std::cout << std::setw(6) << "PID"
                  << std::setw(10) << "CPU%"
                  << std::setw(10) << "MEM%"
                  << "  COMMAND\n";
        std::cout << std::fixed << std::setprecision(1);
        for (size_t j = 0; j < std::min<size_t>(5, procs.size()); ++j) {
            auto& p = procs[j];
            std::cout << std::setw(6) << p.pid
                      << std::setw(10) << p.cpu_percent
                      << std::setw(10) << p.mem_percent
                      << "  " << p.command << "\n";
        }
    }
    return 0;
}