# chat_project (v02)

Проект по C++17. Этапы:
- v01: Message (Rule of Zero), OwningBuffer (Rule of Five), ChatStore на std::map.
- v02: лексер на std::string_view, Command на std::variant, parse через
       std::optional, structured bindings в обходе, типы содержимого
       (TextBody / SystemNotice / Reaction), ASan-тест на висячий string_view.

## Сборка
mkdir build && cd build
cmake ..
cmake --build .

## Запуск
./chat_cli
ctest

## Команды chat_cli (v02)

- `/join <room>`     — создать/войти в комнату
- `/leave <room>`    — выйти из комнаты
- `/msg <room> <текст...>` — отправить сообщение (весь остаток — текст)
- `/who <room>`      — список участников
- `/history <room>`  — история сообщений комнаты
- `/type <room> <index>` — имя альтернативы содержимого (TextBody/SystemNotice/Reaction)
- `/react <room