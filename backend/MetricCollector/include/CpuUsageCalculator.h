#pragma once

#include <vector>

namespace sysmon {

/**
 * @brief Вычисляет загрузку CPU в процентах по двум замерам тиков.
 *
 * Формула: usage = 100 * (1 - Δidle / Δtotal).
 * idle = поле idle + поле iowait.
 * Источник: kernel.org/doc/Documentation/filesystems/proc.txt
 */
class CpuUsageCalculator {
public:
    /**
     * @brief Рассчитать загрузку CPU.
     * @param prev тики первого замера (порядок: user, nice, system, idle, iowait, irq, softirq, steal)
     * @param curr тики второго замера (через интервал)
     * @return Загрузка CPU в процентах [0, 100], или 0.0 при некорректных данных.
     */
    static double calculate(const std::vector<unsigned long long>& prev,
                            const std::vector<unsigned long long>& curr);
};

} // namespace sysmon