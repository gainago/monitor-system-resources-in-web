#include "CpuUsageCalculator.h"

namespace sysmon {

double CpuUsageCalculator::calculate(const std::vector<unsigned long long>& prev,
                                    const std::vector<unsigned long long>& curr) {
    if (prev.empty() || curr.empty() || prev.size() < 8 || curr.size() < 8) {
        return 0.0; // недостаточно данных
    }

    // Индексы: 3=idle, 4=iowait
    auto sum_ticks = [](const std::vector<unsigned long long>& v) {
        unsigned long long total = 0;
        unsigned long long idle = 0;
        for (size_t i = 0; i < v.size(); ++i) {
            total += v[i];
            if (i == 3 || i == 4) {
                idle += v[i];
            }
        }
        return std::make_pair(total, idle);
    };

    auto [prev_total, prev_idle] = sum_ticks(prev);
    auto [curr_total, curr_idle] = sum_ticks(curr);

    auto delta_total = curr_total - prev_total;
    auto delta_idle  = curr_idle  - prev_idle;

    if (delta_total == 0) {
        return 0.0;
    }

    double usage = 100.0 * (1.0 - static_cast<double>(delta_idle) / delta_total);
    if (usage < 0.0) usage = 0.0;
    if (usage > 100.0) usage = 100.0;
    return usage;
}

} // namespace sysmon