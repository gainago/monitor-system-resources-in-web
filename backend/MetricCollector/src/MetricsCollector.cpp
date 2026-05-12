#include "MetricsCollector.h"
#include "CpuUsageCalculator.h"
#include <thread>
#include <chrono>

namespace sysmon {

MetricsCollector::MetricsCollector(std::unique_ptr<IProcReader> reader, unsigned int cpu_core_count)
    : reader_(std::move(reader)), cpu_core_count_(cpu_core_count)
{
    process_collector_ = std::make_unique<ProcessCollector>(reader_, cpu_core_count_);
}

FullSystemSnapshot MetricsCollector::collect() {
    using namespace std::chrono;
    auto now = steady_clock::now();
    if (first_call_) {
        prev_cpu_ticks_ = reader_->readCpuTicks();
        last_collect_time_ = now;
        first_call_ = false;
        std::this_thread::sleep_for(seconds(1));
        now = steady_clock::now();
    } else {
        auto elapsed = now - last_collect_time_;
        if (elapsed < seconds(1)) {
            std::this_thread::sleep_for(seconds(1) - elapsed);
            now = steady_clock::now();
        }
    }

    auto cur_ticks = reader_->readCpuTicks();
    double cpu_raw = CpuUsageCalculator::calculate(prev_cpu_ticks_, cur_ticks);
    double cpu = cpu_raw * cpu_core_count_;
    prev_cpu_ticks_ = cur_ticks;
    last_collect_time_ = now;

    auto mem_total = reader_->readMemTotal();
    auto mem_avail = reader_->readMemAvailable();
    unsigned long long mem_used = (mem_total > mem_avail) ? (mem_total - mem_avail) : 0;
    double ram_percent = (mem_total > 0) ? (100.0 * mem_used / mem_total) : 0.0;
    auto load = reader_->readLoadAvg();
    double uptime = reader_->readUptimeSeconds();

    SystemMetrics sys;
    sys.cpu_percent = cpu;
    sys.ram_used_kb = mem_used;
    sys.ram_total_kb = mem_total;
    sys.ram_percent = ram_percent;
    sys.load_avg = load;
    sys.uptime_seconds = uptime;
    sys.timestamp_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    FullSystemSnapshot snapshot;
    snapshot.system = sys;
    snapshot.processes = process_collector_->collectProcesses(mem_total);
    return snapshot;
}

} // namespace sysmon