#pragma once

#include "FullSystemSnapshot.h"
#include "ProcessInfo.h"
#include <vector>
#include <functional>

namespace sysmon {

/**
 * @brief Поля, по которым можно сортировать процессы.
 */
enum class SortField {
    PID,
    CPU_PERCENT,
    MEM_PERCENT,
    USER,
    TIME_TICKS,
    VIRT_KB,
    RES_KB,
    SHR_KB,
    PRIORITY,
    NICE,
    STATE
};

/**
 * @brief Направление сортировки.
 */
enum class SortOrder {
    ASC,
    DESC
};

/**
 * @brief Утилита для сортировки списка процессов.
 *
 * Метод sort() создаёт новый вектор с отсортированными ProcessInfo,
 * не изменяя оригинал.
 */
class MetricsSorter {
public:
    /**
     * @brief Отсортировать процессы по указанному полю.
     * @param processes Исходный список (константная ссылка).
     * @param field Поле для сортировки.
     * @param order Направление.
     * @return Новый вектор с отсортированными элементами.
     */
    static std::vector<ProcessInfo> sort(const std::vector<ProcessInfo>& processes,
                                         SortField field = SortField::CPU_PERCENT,
                                         SortOrder order = SortOrder::DESC);
};

} // namespace sysmon