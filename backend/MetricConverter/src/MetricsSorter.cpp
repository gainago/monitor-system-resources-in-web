#include "MetricsSorter.h"
#include <algorithm>

namespace sysmon {

std::vector<ProcessInfo> MetricsSorter::sort(const std::vector<ProcessInfo>& processes,
                                             SortField field, SortOrder order) {
    std::vector<ProcessInfo> result = processes;

    auto compare = [field, order](const ProcessInfo& a, const ProcessInfo& b) -> bool {
        // Функция извлечения значения по полю (double для числовых, std::string для строк)
        auto get_val = [field](const ProcessInfo& p) -> double {
            switch (field) {
                case SortField::PID:          return static_cast<double>(p.pid);
                case SortField::CPU_PERCENT:  return p.cpu_percent;
                case SortField::MEM_PERCENT:  return p.mem_percent;
                case SortField::TIME_TICKS:   return static_cast<double>(p.time_ticks);
                case SortField::VIRT_KB:      return static_cast<double>(p.virt_kb);
                case SortField::RES_KB:       return static_cast<double>(p.res_kb);
                case SortField::SHR_KB:       return static_cast<double>(p.shr_kb);
                case SortField::PRIORITY:     return static_cast<double>(p.priority);
                case SortField::NICE:         return static_cast<double>(p.nice);
                case SortField::STATE:        return static_cast<double>(p.state); // символ -> ASCII
                default: return 0.0;
            }
        };

        // Для USER отдельная ветка
        if (field == SortField::USER) {
            if (a.user == b.user) return false;
            return (order == SortOrder::ASC) ? (a.user < b.user) : (a.user > b.user);
        }

        double va = get_val(a);
        double vb = get_val(b);
        if (va == vb) return false;
        return (order == SortOrder::ASC) ? (va < vb) : (va > vb);
    };

    std::sort(result.begin(), result.end(), compare);
    return result;
}

} // namespace sysmon