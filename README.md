# Satellite Tracker :satellite: Трекер спутников

![interface](docs/SatelliteTracker-1.0.0.png)

## Описание

Satellite Tracker - это программа для отслеживания положения спутников в реальном времени. 

Некоторые возможности:

- Отслеживание положения спутников (с возможностью отключения их отображения).
- Наблюдение траектории движения за некоторый промежуток времени (с возможностью отключения ее отображения).
- Фильтрация по назначению или высоте орбиты.
- Выбор цвета для отображения спутников и их орбит.

## Сборка

### Требования

- CMake версии 3.14 или выше

- Компилятор с поддержкой C++20 (MSVC, GCC, Clang)

- vcpkg для управления зависимостями

- Git (для клонирования репозитория и подмодулей)

#### Установка vcpkg

1. Клонируйте vcpkg:

    ``` git clone https://github.com/Microsoft/vcpkg.git ```
2. Запустите скрипт настройки:

- Windows (PowerShell):

   ``` .\vcpkg\bootstrap-vcpkg.bat ```

- Linux/macOS:

    ``` ./vcpkg/bootstrap-vcpkg.sh ```

#### Клонирование проекта
```
git clone https://github.com/your-username/SatelliteTracker.git
cd SatelliteTracker
git submodule update --init --recursive
```
### Сборка проекта
- Windows
```
# Генерация проектов Visual Studio
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake

# Сборка
cmake --build build --config Release
```
- Linux
```
# Генерация Makefile
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake

# Сборка
cmake --build build --config Release
Замените [path/to/vcpkg] на полный путь к вашей директории vcpkg.
```

### Запуск
После успешной сборки исполняемый файл будет находиться в папке ``` build/bin/ ```