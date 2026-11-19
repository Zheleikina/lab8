#pragma once

#include <string>

// Основна сутність: пакет зображень, що надходить на конвеєр обробки.
struct ImageBatch
{
    int id = 0;
    std::string name;
    std::string sourcePath;
    std::string status;      // queued | processing | completed | failed
    std::string createdDate; // формат YYYY-MM-DD
};
