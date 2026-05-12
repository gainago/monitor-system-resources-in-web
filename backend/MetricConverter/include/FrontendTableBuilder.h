#pragma once

#include "FullSystemSnapshot.h"
#include <string>

namespace sysmon {

/**
 * @brief Строит JSON-представление для отображения на фронтенде.
 *
 * Включает системные метрики и таблицу процессов с заголовками колонок
 * и отформатированными значениями (обрезка команд, округление CPU/MEM,
 * форматирование времени).
 */
class FrontendTableBuilder {
public:
    /**
     * @brief Построить JSON строку по полному снимку системы.
     * @param snapshot Снимок от MetricsCollector.
     * @return std::string с JSON.
     */
    static std::string build(const FullSystemSnapshot& snapshot);
};

} // namespace sysmon