# Покер - Консольная игра

Консольная игра в покер на C++ с поддержкой игры против ботов.

## Структура проекта

```
Игра → Колода → Игроки / Игровое поле
Игрок → Кошелёк → Банк
Игроки / Игрок-бот → История ставок
Игровое поле → Оценщик комбинаций → Результат
Результат → Менеджер состояния
Игра ↔ Таймер
Игра ↔ Менеджер состояния
```

## Компиляция

### Требования
- Компилятор C++17 (g++, clang++, или MSVC)
- CMake (опционально)

### Используя g++/clang
```bash
g++ -std=c++17 -Wall -Iinclude -o poker.exe src/main.cpp src/Wallet.cpp src/Bank.cpp src/BetHistory.cpp src/Player.cpp src/BotPlayer.cpp src/HumanPlayer.cpp src/Deck.cpp src/HandEvaluator.cpp src/GameBoard.cpp src/Result.cpp src/StateManager.cpp src/Timer.cpp src/Game.cpp
```

### Используя CMake
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Windows (MSVC)
```cmd
cl /EHsc /std:c++17 /I include src\*.cpp /Fe:poker.exe
```

## Запуск
```bash
# Linux/Mac
./poker

# Windows
poker.exe
```

## Особенности

- **Игра против ботов**: Играйте против двух AI-противников
- **Полная механика покера**: Префлоп, Флоп, Тёрн, Ривер, Шоудаун
- **Оценка комбинаций**: Автоматическая оценка и сравнение рук
- **Банк и ставки**: Реалистичная система ставок
- **История ставок**: Отслеживание всех ставок в раздаче

## Классы

- `Card` - Карта
- `Wallet` - Кошелёк игрока
- `Bank` - Банк для управления ставками
- `BetHistory` - История ставок
- `Player` - Базовый класс игрока
- `HumanPlayer` - Игрок-человек
- `BotPlayer` - Игрок-бот
- `Deck` - Колода карт
- `HandEvaluator` - Оценщик покерных комбинаций
- `GameBoard` - Игровое поле
- `Result` - Результат раздачи
- `StateManager` - Менеджер состояния игры
- `Timer` - Таймер для ходов
- `Game` - Главный класс игры

## Лицензия

См. файл LICENSE
