#pragma once

#include <string>

// Залежна сутність: завдання обробки, що виконується над конкретним пакетом зображень.
struct ProcessingTask
{
    int id = 0;
    int batchId = 0;           // зовнішній ключ на ImageBatch.id
    std::string operationType; // resize | filter | compress | watermark
    std::string status;        // pending | completed | failed
    double durationMs = 0.0;   // числове поле для агрегатних обчислень
    std::string processedDate; // формат YYYY-MM-DD
};
