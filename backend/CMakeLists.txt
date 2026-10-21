cmake_minimum_required(VERSION 3.21)
project(MegapolisBackend VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Если в бэкенде используются QString / QJson / QFile / QObject — нужен Qt Core.
# Если чистый STL — уберите find_package и target_link_libraries ниже.
find_package(Qt6 6.5 REQUIRED COMPONENTS Core)

add_library(backend STATIC
    # корень
    config.cpp
    config.h

    # building
    building/buildings.cpp
    building/buildings.h

    # core
    core/GameEngine.cpp
    core/GameEngine.h
    core/GameState.cpp
    core/GameState.h

    # interaction
    interaction/Decision.cpp
    interaction/Decision.h
    interaction/offer.cpp
    interaction/offer.h

    # modules
    modules/District.cpp
    modules/District.h
    modules/RealtorAgency.cpp
    modules/RealtorAgency.h
    modules/RulesValidator.cpp
    modules/RulesValidator.h
    modules/StrategyUtils.cpp
    modules/StrategyUtils.h

    # player
    player/Player.cpp
    player/Player.h
    player/BalancedPlayer.cpp
    player/BalancedPlayer.h
    player/BuilderPlayer.cpp
    player/BuilderPlayer.h
    player/MarketPlayer.cpp
    player/MarketPlayer.h
    player/RandomPlayer.cpp
    player/RandomPlayer.h

    # view
    view/PublicView.cpp
    view/PublicView.h
)

# Позволяет фронтенду писать #include "core/GameEngine.h"
target_include_directories(backend PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(backend PUBLIC Qt6::Core)

target_compile_options(backend PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic>
)
