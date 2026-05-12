#pragma once

#include <array>
#include <cstdint>

namespace sysmon {

/**
 * @brief Структура-снимок системных метрик для передачи потребителю.
 *
 * Содержит все показатели, сериализуемые в JSON для фронтенда.
 */
struct SystemMetrics {
    double cpu_percent = 0.0;               ///< Загрузка CPU в процентах (0-100)
    unsigned long long ram_used_kb = 0;     ///< Использовано ОЗУ, КБ
    unsigned long long ram_total_kb = 0;    ///< Всего ОЗУ, КБ
    double ram_percent = 0.0;               ///< Процент занятой ОЗУ
    std::array<double, 3> load_avg{0.0, 0.0, 0.0}; ///< Load avg за 1,5,15 мин
    double uptime_seconds = 0.0;            ///< Время работы системы, секунд
    long long timestamp_ms = 0;            ///< UNIX-время в миллисекундах
};

} // namespace sysmon