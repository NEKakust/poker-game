cmake_minimum_required(VERSION 3.10)
project(poker_game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Настройки для Visual Studio
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD ON)

# Список всех исходных файлов
set(SOURCES
    poker/main.cpp
    poker/Player.cpp
    poker/Card.cpp
    poker/Deck.cpp
    poker/PokerGame.cpp
    poker/Bank.cpp
    poker/BetHistory.cpp
    poker/BotPlayer.cpp
    poker/GameBoard.cpp
    poker/HandEvaluator.cpp
    poker/Result.cpp
    poker/StateManager.cpp
    poker/Wallet.cpp
)

# Список всех заголовочных файлов
set(HEADERS
    poker/Player.h
    poker/Card.h
    poker/Deck.h
    poker/PokerGame.h
    poker/Bank.h
    poker/BetHistory.h
    poker/BotPlayer.h
    poker/GameBoard.h
    poker/HandEvaluator.h
    poker/Result.h
    poker/StateManager.h
    poker/Wallet.h
)

# Добавляем исполняемый файл
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Включаем директорию poker для поиска заголовков
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/poker
)

# Отключаем предварительную компиляцию
set_target_properties(${PROJECT_NAME} PROPERTIES
    ENABLE_PRECOMPILED_HEADERS OFF
)

# Отключаем макросы min/max из Windows.h для избежания конфликтов
if(MSVC)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Компилируем с UTF-8 для корректного отображения русского текста
    add_compile_options(/utf-8)
endif()

