# chat_project (v01, C++14)

Учебный проект на C++14. Демонстрирует:
- правило нуля (Message);
- правило пяти и copy-and-swap (OwningBuffer);
- move-семантику;
- лямбды с захватом по ссылке и по значению;
- GoogleTest через FetchContent.

## Сборка
mkdir build && cd build
cmake ..
cmake --build .


## Запуск
./chat_cli # консольный клиент
ctest # тесты
./chat_tests # тесты напрямую


## Команды chat_cli

- `/post <room> <author> <text>`
- `/history <room>`
- `/exit`

## Примечание про стандарт

Проект собирается под `-std=c++14`. Все конструкции (Rule of Zero/Five,
`std::function`, лямбды с `mutable`, `noexcept`, `std::move`) доступны
в C++14 без изменений. `std::string::starts_with` отсутствует — заменён
на вспомогательную функцию через `std::string::compare`.