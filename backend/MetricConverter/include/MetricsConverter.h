#pragma once

#include "FullSystemSnapshot.h"
#include <string>

namespace sysmon {

/**
 * @brief Перечисление полей для сортировки списка процессов.
 */
enum class SortField {
    PID,
    CPU_PERCENT,
    MEM_PERCENT,
    USER,
    // можно добавить другие поля при необходимости
};

/**
 * @brief Порядок сортировки.
 */
enum class SortOrder {
    ASC,
    DESC
};

/**
 * @brief Класс-преобразователь снимка системы в JSON с сортировкой процессов.
 *
 * Отделяет представление (сортировку и сериализацию) от сбора данных.
 * Позволяет задать поле и направление сортировки.
 */
class MetricsConverter {
public:
    /**
     * @brief Конструктор по умолчанию.
     */
    MetricsConverter() = default;

    /**
     * @brief Преобразовать полный снимок системы в JSON-строку.
     *
     * @param snapshot Снимок от MetricsCollector.
     * @param field Поле для сортировки процессов (по умолчанию CPU_PERCENT).
     * @param order Направление сортировки (по умолчанию DESC – от большего к меньшему).
     * @return std::string с JSON.
     */
    std::string toJson(const FullSystemSnapshot& snapshot,
                       SortField field = SortField::CPU_PERCENT,
                       SortOrder order = SortOrder::DESC) const;

private:
    /**
     * @brief Вспомогательная функция сравнения двух процессов.
     */
    static bool compareProcesses(const ProcessInfo& a, const ProcessInfo& b,
                                 SortField field, SortOrder order);
};

} // namespace sysmon