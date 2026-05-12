#include "MetricsCollector.h"
#include "ProcReader.h"
#include "MetricsSorter.h"
#include "FrontendTableBuilder.h"
#include <iostream>
#include <iomanip>

int main() {
    using namespace sysmon;

    auto reader = std::make_unique<ProcReader>();
    unsigned int cores = reader->getCpuCount();
    std::cout << "Detected " << cores << " CPU cores\n";

    MetricsCollector collector(std::move(reader), cores);

    // Два замера для появления CPU% у процессов
    collector.collect(); 
    for (int i{0}; i < 5; i++) {                         // сброс кэша
        FullSystemSnapshot snapshot = collector.collect(); // актуальные данные

        // Сортировка по CPU% убыванию
        std::vector<ProcessInfo> sorted_procs = MetricsSorter::sort(snapshot.processes,
                                                                    SortField::CPU_PERCENT,
                                                                    SortOrder::DESC);
        snapshot.processes = sorted_procs; // заменить в снимке

        // Построить JSON для фронта
        std::string json = FrontendTableBuilder::build(snapshot);

        // Вывести для проверки
        std::cout << "Frontend JSON:\n" << json.substr(0, 5000) << "...\n\n";

        // Покажем в консоли топ-5 (простым форматированием)
        std::cout << "\n=== Console top-5 ===\n";
        std::cout << std::left  << std::setw(6)  << "PID"
                << std::right << std::setw(8)  << "CPU%"
                << std::right << std::setw(8)  << "MEM%"
                << std::left  << "  COMMAND\n";
        for (size_t i = 0; i < std::min<size_t>(5, snapshot.processes.size()); ++i) {
            auto& p = snapshot.processes[i];
            std::cout << std::left  << std::setw(6)  << p.pid
                    << std::right << std::setw(8)  << std::fixed << std::setprecision(1) << p.cpu_percent
                    << std::right << std::setw(8)  << std::fixed << std::setprecision(1) << p.mem_percent
                    << std::left  << "  " << (p.command.size() > 50 ? p.command.substr(0, 47) + "..." : p.command) << "\n";
        }
    }

    return 0;
}