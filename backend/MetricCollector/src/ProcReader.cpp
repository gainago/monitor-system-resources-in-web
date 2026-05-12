#include "ProcReader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

namespace sysmon {

static const char* const PROC_STAT     = "/proc/stat";
static const char* const PROC_MEMINFO  = "/proc/meminfo";
static const char* const PROC_LOADAVG  = "/proc/loadavg";
static const char* const PROC_UPTIME   = "/proc/uptime";

std::vector<unsigned long long> ProcReader::readCpuTicks() {
    std::ifstream stat(PROC_STAT);
    if (!stat) {
        std::cerr << "[ProcReader] Failed to open " << PROC_STAT << std::endl;
        return {};
    }

    std::string line;
    std::getline(stat, line); // первая строка: "cpu  ..."
    std::istringstream iss(line);

    std::string cpu_label;
    iss >> cpu_label; // пропускаем "cpu"
    if (cpu_label != "cpu") {
        std::cerr << "[ProcReader] Unexpected format in " << PROC_STAT << std::endl;
        return {};
    }

    std::vector<unsigned long long> ticks;
    unsigned long long val;
    while (iss >> val) {
        ticks.push_back(val);
    }

    if (ticks.size() < 8) { // минимально ожидается 8 полей
        std::cerr << "[ProcReader] Not enough CPU fields in " << PROC_STAT << std::endl;
        return {};
    }
    return ticks;
}

unsigned long long ProcReader::readMemTotal() {
    std::ifstream meminfo(PROC_MEMINFO);
    if (!meminfo) {
        std::cerr << "[ProcReader] Failed to open " << PROC_MEMINFO << std::endl;
        return 0;
    }
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.rfind("MemTotal:", 0) == 0) {
            std::istringstream iss(line);
            std::string label;
            unsigned long long value;
            std::string unit;
            iss >> label >> value >> unit; // "MemTotal: 16384000 kB"
            return value; // в килобайтах
        }
    }
    std::cerr << "[ProcReader] MemTotal not found in " << PROC_MEMINFO << std::endl;
    return 0;
}

unsigned long long ProcReader::readMemAvailable() {
    std::ifstream meminfo(PROC_MEMINFO);
    if (!meminfo) {
        std::cerr << "[ProcReader] Failed to open " << PROC_MEMINFO << std::endl;
        return 0;
    }
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.rfind("MemAvailable:", 0) == 0) {
            std::istringstream iss(line);
            std::string label;
            unsigned long long value;
            std::string unit;
            iss >> label >> value >> unit;
            return value;
        }
    }
    std::cerr << "[ProcReader] MemAvailable not found in " << PROC_MEMINFO << std::endl;
    return 0;
}

std::array<double, 3> ProcReader::readLoadAvg() {
    std::ifstream loadavg(PROC_LOADAVG);
    if (!loadavg) {
        std::cerr << "[ProcReader] Failed to open " << PROC_LOADAVG << std::endl;
        return {0.0, 0.0, 0.0};
    }
    double one, five, fifteen;
    if (!(loadavg >> one >> five >> fifteen)) {
        std::cerr << "[ProcReader] Failed to parse " << PROC_LOADAVG << std::endl;
        return {0.0, 0.0, 0.0};
    }
    return {one, five, fifteen};
}

double ProcReader::readUptimeSeconds() {
    std::ifstream uptime(PROC_UPTIME);
    if (!uptime) {
        std::cerr << "[ProcReader] Failed to open " << PROC_UPTIME << std::endl;
        return 0.0;
    }
    double uptime_secs;
    if (!(uptime >> uptime_secs)) {
        std::cerr << "[ProcReader] Failed to parse " << PROC_UPTIME << std::endl;
        return 0.0;
    }
    return uptime_secs;
}

unsigned int ProcReader::getCpuCount() {
    std::ifstream stat(PROC_STAT);
    if (!stat) {
        std::cerr << "[ProcReader] Failed to open " << PROC_STAT << " for cpu count\n";
        return 1; // фолбек на 1 ядро
    }
    std::string line;
    unsigned int count = 0;
    while (std::getline(stat, line)) {
        // строки с информацией о конкретном ядре начинаются с "cpu" и дальше идёт число
        if (line.rfind("cpu", 0) == 0 && line.size() > 3 && std::isdigit(line[3])) {
            ++count;
        }
    }
    return (count > 0) ? count : 1;
}

std::vector<int> ProcReader::getPids() {
    std::vector<int> pids;
    DIR* dir = opendir("/proc");
    if (!dir) {
        std::cerr << "[ProcReader] Cannot open /proc\n";
        return pids;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            const char* name = entry->d_name;
            bool is_digit = true;
            for (; *name; ++name) {
                if (!std::isdigit(*name)) {
                    is_digit = false;
                    break;
                }
            }
            if (is_digit && strlen(entry->d_name) > 0) {
                pids.push_back(std::stoi(entry->d_name));
            }
        }
    }
    closedir(dir);
    return pids;
}

std::string ProcReader::resolveUid(unsigned int uid) {
    auto it = uid_cache_.find(uid);
    if (it != uid_cache_.end()) return it->second;
    struct passwd* pw = getpwuid(uid);
    std::string name = pw ? pw->pw_name : std::to_string(uid);
    uid_cache_[uid] = name;
    return name;
}

ProcessInfo ProcReader::readProcessInfo(int pid) {
    ProcessInfo info;
    info.pid = pid;

    // Читаем /proc/pid/stat
    std::string stat_path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream stat_file(stat_path);
    if (!stat_file) return info;

    std::string stat_line;
    std::getline(stat_file, stat_line);
    // Парсинг stat: сложная строка из-за comm в скобках
    // Найдём позицию '(' и ')'
    auto open_br = stat_line.find('(');
    auto close_br = stat_line.rfind(')');
    if (open_br == std::string::npos || close_br == std::string::npos) return info;

    // Извлекаем comm (в скобках) для команды
    std::string comm = stat_line.substr(open_br + 1, close_br - open_br - 1);

    // Всё после ')' — оставшиеся поля, начинающиеся с state
    std::istringstream rest(stat_line.substr(close_br + 2));
    // Поля: state ppid pgrp session tty_nr tpgid flags minflt cminflt majflt cmajflt
    //       utime stime cutime cstime priority nice num_threads itrealvalue starttime vsize rss ...
    char state;
    rest >> state;
    info.state = state;

    long long dummy;
    // пропускаем 9 полей (ppid...cmajflt) — всего их 11 после state, но проще считать индексы
    for (int i = 0; i < 10; ++i) rest >> dummy; // ppid..cmajflt

    unsigned long long utime, stime;
    rest >> utime >> stime;

    // cutime, cstime пропускаем (2 поля)
    rest >> dummy >> dummy;

    long priority, nice_val;
    rest >> priority >> nice_val;
    info.priority = priority;
    info.nice = nice_val;

    // num_threads, itrealvalue пропускаем
    rest >> dummy >> dummy;

    unsigned long long starttime, vsize, rss;
    rest >> starttime >> vsize >> rss;

    long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;
    info.virt_kb = vsize / 1024;  // vsize в байтах -> КБ
    info.res_kb = rss * page_size_kb;
    info.time_ticks = utime + stime;

    // Читаем /proc/pid/status для UID и RssShmem
    std::string status_path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream status_file(status_path);
    if (status_file) {
        std::string line;
        while (std::getline(status_file, line)) {
            if (line.rfind("Uid:", 0) == 0) {
                // Uid:   real   effective   saved   filesystem
                std::istringstream iss(line);
                std::string label;
                unsigned int uid_real;
                iss >> label >> uid_real;
                info.user = resolveUid(uid_real);
            } else if (line.rfind("RssShmem:", 0) == 0) {
                std::istringstream iss(line);
                std::string label;
                unsigned long long val_kb;
                iss >> label >> val_kb;
                info.shr_kb = val_kb; // в status уже в КБ
            }
        }
    }

    // Читаем cmdline для command
    std::string cmdline_path = "/proc/" + std::to_string(pid) + "/cmdline";
    std::ifstream cmdline_file(cmdline_path);
    if (cmdline_file) {
        std::string cmdline;
        std::getline(cmdline_file, cmdline, '\0'); // читаем до нуля, но лучше весь файл
        // Заменим нулевые байты на пробелы
        std::string command_str;
        for (char c : cmdline) {
            if (c == '\0') command_str += ' ';
            else command_str += c;
        }
        if (!command_str.empty()) {
            info.command = command_str;
        } else {
            info.command = comm; // fallback к comm из stat
        }
    } else {
        info.command = comm;
    }

    return info;
}

unsigned long long ProcReader::getTotalCpuTicks() {
    auto ticks = readCpuTicks(); // это первая строка cpu
    if (ticks.empty()) return 0;
    unsigned long long total = 0;
    for (auto t : ticks) total += t;
    return total;
}

} // namespace sysmon