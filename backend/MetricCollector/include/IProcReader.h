#pragma once

#include <vector>
#include <array>
#include <string>
#include "ProcessInfo.h"

namespace sysmon {

class IProcReader {
public:
    virtual ~IProcReader() = default;

    // Существующие методы
    virtual std::vector<unsigned long long> readCpuTicks() = 0;
    virtual unsigned long long readMemTotal() = 0;
    virtual unsigned long long readMemAvailable() = 0;
    virtual std::array<double, 3> readLoadAvg() = 0;
    virtual double readUptimeSeconds() = 0;
    virtual unsigned int getCpuCount() = 0;

    // Новые методы для процессов
    /**
     * @brief Получить список PID всех процессов в системе.
     */
    virtual std::vector<int> getPids() = 0;

    /**
     * @brief Прочитать информацию о процессе по PID.
     * @return Заполненная структура ProcessInfo (кроме cpu_percent и mem_percent).
     *         Поля, которые не удалось прочитать, остаются нулевыми.
     */
    virtual ProcessInfo readProcessInfo(int pid) = 0;

    /**
     * @brief Получить сумму всех тиков CPU из /proc/stat (total).
     */
    virtual unsigned long long getTotalCpuTicks() = 0;
};

} // namespace sysmon