# Image Pipeline ORM — Лабораторна робота 8 (варіант 3)

Консольний застосунок C++20 для предметної області «Конвеєр обробки зображень».

- Основна сутність: **ImageBatch** (пакет зображень).
- Залежна сутність: **ProcessingTask** (завдання обробки), зв'язок 1:N через `batchId`.

## Збірка

Потрібні: компілятор C++20 (MSVC), CMake, vcpkg (manifest mode).

```bash
cmake --preset default
cmake --build build
```

Переконайтесь, що встановлено змінну середовища `VCPKG_ROOT`, яка вказує на
корінь встановленого vcpkg (використовується в `CMakePresets.json`).

## Запуск

```bash
./build/ImagePipelineORM.exe
```

Програма послідовно демонструє:

1. Операції через "сирий" SQLite C API (`RawSqliteDemo`).
2. Базові CRUD-операції через `sqlite_orm`.
3. Зв'язок 1:N між `ImageBatch` і `ProcessingTask`, перевірку зовнішнього ключа.
4. Патерн Repository (`IImageBatchRepository` / `SQLiteImageBatchRepository`).
5. Фільтрацію, сортування та обмеження кількості результатів.
6. Агрегатні запити (COUNT, SUM, AVG, GROUP BY).
7. Об'єднання таблиць (INNER JOIN).
8. Успішну транзакцію (COMMIT), відкат (ROLLBACK) та порівняння з виконанням
   без явної транзакції.

Повторні запуски не накопичують тестові дані: демонстраційні БД для запитів
(`image_pipeline_queries.db`, `image_pipeline_no_transaction.db`) перестворюються
кожного разу.
