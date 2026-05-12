#include "MetricsConverter.h"
#include "json.hpp"
#include <algorithm>

namespace sysmon {

bool MetricsConverter::compareProcesses(const ProcessInfo& a, const ProcessInfo& b,
                                        SortField field, SortOrder order) {
    bool less = false;
    switch (field) {
        case SortField::PID:
            less = a.pid < b.pid;
            break;
        case SortField::CPU_PERCENT:
            less = a.cpu_percent < b.cpu_percent;
            break;
        case SortField::MEM_PERCENT:
            less = a.mem_percent < b.mem_percent;
            break;
        case SortField::USER:
            less = a.user < b.user;
            break;
        default:
            less = a.pid < b.pid;
    }
    return (order == SortOrder::ASC) ? less : !less;
}

std::string MetricsConverter::toJson(const FullSystemSnapshot& snapshot,
                                    SortField field,
                                    SortOrder order) const {
    using json = nlohmann::json;

    // Системные метрики
    json j;
    j["system"]["cpu_percent"] = snapshot.system.cpu_percent;
    j["system"]["ram_used_kb"] = snapshot.system.ram_used_kb;
    j["system"]["ram_total_kb"] = snapshot.system.ram_total_kb;
    j["system"]["ram_percent"] = snapshot.system.ram_percent;
    j["system"]["load_avg"] = snapshot.system.load_avg;
    j["system"]["uptime_seconds"] = snapshot.system.uptime_seconds;
    j["system"]["timestamp_ms"] = snapshot.system.timestamp_ms;

    // Копируем список процессов для сортировки (не изменяем оригинал)
    std::vector<ProcessInfo> sorted = snapshot.processes;
    std::sort(sorted.begin(), sorted.end(),
              [field, order](const ProcessInfo& a, const ProcessInfo& b) {
                  return compareProcesses(a, b, field, order);
              });

    json processes_array = json::array();
    for (const auto& proc : sorted) {
        json p;
        p["pid"] = proc.pid;
        p["user"] = proc.user;
        p["priority"] = proc.priority;
        p["nice"] = proc.nice;
        p["virt_kb"] = proc.virt_kb;
        p["res_kb"] = proc.res_kb;
        p["shr_kb"] = proc.shr_kb;
        p["state"] = std::string(1, proc.state);
        p["cpu_percent"] = proc.cpu_percent;
        p["mem_percent"] = proc.mem_percent;
        p["time_ticks"] = proc.time_ticks;
        p["command"] = proc.command;
        processes_array.push_back(p);
    }
    j["processes"] = processes_array;

    return j.dump();
}

} // namespace sysmon