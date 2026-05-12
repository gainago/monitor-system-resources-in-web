#pragma once

#include <string>

namespace sysmon {

/**
 * @brief Данные одного процесса для отображения в таблице (аналог htop).
 */
struct ProcessInfo {
    int pid = 0;
    std::string user;
    int priority = 0;        // PR
    int nice = 0;            // NI
    unsigned long long virt_kb = 0; // VIRT
    unsigned long long res_kb = 0;  // RES
    unsigned long long shr_kb = 0;  // SHR (RssShmem)
    char state = '?';        // S (R/S/D/...)
    double cpu_percent = 0.0;
    double mem_percent = 0.0;
    unsigned long long time_ticks = 0; // utime+stime в тиках (для TIME+)
    std::string command;
};

} // namespace sysmon