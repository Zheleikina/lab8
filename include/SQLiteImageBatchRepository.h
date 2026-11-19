#pragma once

#include <string>

#include "IImageBatchRepository.h"
#include "Database.h"

// Реалізація Repository для ImageBatch через sqlite_orm.
class SQLiteImageBatchRepository : public IImageBatchRepository
{
public:
    explicit SQLiteImageBatchRepository(const std::string& databaseFile);

    int add(const ImageBatch& batch) override;

    std::optional<ImageBatch> findById(int id) override;
    std::vector<ImageBatch> findAll() override;

    bool update(const ImageBatch& batch) override;
    bool remove(int id) override;

    // Доступ до сховища потрібен демонстраційному коду для роботи
    // із залежною сутністю ProcessingTask та для транзакцій.
    Storage& storage();

private:
    Storage storage_;
};
