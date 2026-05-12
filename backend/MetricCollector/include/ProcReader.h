#pragma once

#include "IProcReader.h"
#include <unordered_map>

namespace sysmon {

class ProcReader : public IProcReader {
public:
    // Старые методы
    std::vector<unsigned long long> readCpuTicks() override;
    unsigned long long readMemTotal() override;
    unsigned long long readMemAvailable() override;
    std::array<double, 3> readLoadAvg() override;
    double readUptimeSeconds() override;
    unsigned int getCpuCount() override;

    // Новые методы
    std::vector<int> getPids() override;
    ProcessInfo readProcessInfo(int pid) override;
    unsigned long long getTotalCpuTicks() override;

private:
    // Кэш для преобразования UID -> username
    std::unordered_map<unsigned int, std::string> uid_cache_;
    std::string resolveUid(unsigned int uid);
};

} // namespace sysmon