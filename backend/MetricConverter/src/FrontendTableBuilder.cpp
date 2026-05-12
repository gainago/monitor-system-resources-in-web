#include "FrontendTableBuilder.h"
#include "json.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace sysmon {

static std::string formatTimeTicks(unsigned long long ticks) {
    unsigned long long total_cs = ticks;
    unsigned long long mins = total_cs / (100 * 60);
    unsigned long long secs = (total_cs / 100) % 60;
    unsigned long long hs = total_cs % 100;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << mins << ':'
        << std::setw(2) << secs << '.'
        << std::setw(2) << hs;
    return oss.str();
}

static std::string truncate_command(const std::string& cmd, size_t max_len = 50) {
    if (cmd.size() <= max_len) return cmd;
    return cmd.substr(0, max_len - 3) + "...";
}

static double round_one_decimal(double val) {
    return std::round(val * 10.0) / 10.0;
}

std::string FrontendTableBuilder::build(const FullSystemSnapshot& snapshot) {
    using json = nlohmann::json;
    json j;

    // Система
    j["system"]["cpu_percent"] = snapshot.system.cpu_percent;
    j["system"]["ram_used_kb"] = snapshot.system.ram_used_kb;
    j["system"]["ram_total_kb"] = snapshot.system.ram_total_kb;
    j["system"]["ram_percent"] = snapshot.system.ram_percent;
    j["system"]["load_avg"] = snapshot.system.load_avg;
    j["system"]["uptime_seconds"] = snapshot.system.uptime_seconds;
    j["system"]["timestamp_ms"] = snapshot.system.timestamp_ms;

    // Определение колонок (порядок как в htop)
    json columns = json::array();
    columns.push_back({{"key", "pid"}, {"label", "PID"}});
    columns.push_back({{"key", "user"}, {"label", "USER"}});
    columns.push_back({{"key", "priority"}, {"label", "PR"}});
    columns.push_back({{"key", "nice"}, {"label", "NI"}});
    columns.push_back({{"key", "virt_kb"}, {"label", "VIRT"}});
    columns.push_back({{"key", "res_kb"}, {"label", "RES"}});
    columns.push_back({{"key", "shr_kb"}, {"label", "SHR"}});
    columns.push_back({{"key", "state"}, {"label", "S"}});
    columns.push_back({{"key", "cpu_percent"}, {"label", "CPU%"}});
    columns.push_back({{"key", "mem_percent"}, {"label", "MEM%"}});
    columns.push_back({{"key", "time_ticks"}, {"label", "TIME+"}});
    columns.push_back({{"key", "command"}, {"label", "Command"}});

    json rows = json::array();
    for (const auto& proc : snapshot.processes) {
        json row;
        row["pid"] = proc.pid;
        row["user"] = proc.user;
        row["priority"] = proc.priority;
        row["nice"] = proc.nice;
        // Размеры оставляем в КБ, фронт сам форматирует (или можно здесь преобразовать в строку)
        row["virt_kb"] = proc.virt_kb;
        row["res_kb"] = proc.res_kb;
        row["shr_kb"] = proc.shr_kb;
        row["state"] = std::string(1, proc.state);
        row["cpu_percent"] = round_one_decimal(proc.cpu_percent);
        row["mem_percent"] = round_one_decimal(proc.mem_percent);
        row["time_ticks"] = formatTimeTicks(proc.time_ticks);
        row["command"] = truncate_command(proc.command);
        rows.push_back(row);
    }

    j["table"]["columns"] = columns;
    j["table"]["rows"] = rows;

    return j.dump(); // компактный JSON, для отладки можно j.dump(2)
}

} // namespace sysmon