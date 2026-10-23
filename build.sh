# Использование:
#   ./build.sh

set -e 

if [ ! -f "CMakeLists.txt" ]; then
    echo "Ошибка: CMakeLists.txt не найден."
    echo "Запустите скрипт из корня проекта."
    exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
    echo "Ошибка: cmake не найден в PATH."
    exit 1
fi

mkdir -p build
cd build

cmake ..
cmake --build . -- -j"$(nproc 2>/dev/null || echo 2)"
