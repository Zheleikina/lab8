#pragma once

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "Database.h"
#include "ProcessingTask.h"

// Компонент доступу до даних для залежної сутності ProcessingTask (зв'язок 1:N).
class ProcessingTaskRepository
{
public:
    explicit ProcessingTaskRepository(const std::string& databaseFile);

    int add(const ProcessingTask& task);
    std::vector<ProcessingTask> findByBatchId(int batchId);
    std::vector<ProcessingTask> findAll();

    // Вибірки та агрегати по ProcessingTask
    std::vector<ProcessingTask> findTopLongest(int limitCount);
    std::vector<ProcessingTask> findByOperationSortedByDate(const std::string& operationType);

    int countTasks();
    double totalDurationMs();
    double averageDurationMs();
    std::vector<std::pair<int, double>> totalDurationByBatch();

    // Об'єднання ImageBatch та ProcessingTask (INNER JOIN)
    std::vector<std::tuple<std::string, std::string, std::string, double>> getHistoryWithBatchName();
    std::vector<std::tuple<std::string, std::string, double>> getTasksLongerThan(double minDurationMs);

private:
    Storage storage_;
};