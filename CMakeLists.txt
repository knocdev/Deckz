cmake_minimum_required(VERSION 3.20)
project(CardEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
    DOWNLOAD_NO_EXTRACT TRUE
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/include/nlohmann
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(engine
    src/CardAttribute.cpp
    src/CardType.cpp
    src/Card.cpp
    src/DeckType.cpp
    src/Deck.cpp
    src/Phase.cpp
    src/Registry.cpp
    src/Hand.cpp
    src/Player.cpp
    src/TurnEngine.cpp
    src/GameState.cpp
    src/ActionValidator.cpp
    src/EffectRegistry.cpp
    src/Game.cpp
)

target_include_directories(engine
    PUBLIC  include
    PRIVATE ${CMAKE_BINARY_DIR}/include
)

# --- deckz CLI tool ---
if(WIN32)
    set(DECKZ_PLATFORM_LIBS "-lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32")
else()
    set(DECKZ_PLATFORM_LIBS "")
endif()

configure_file(
    tools/deckz/deckz_config.h.in
    ${CMAKE_BINARY_DIR}/deckz_config.h
)

add_executable(deckz tools/deckz/main.cpp)
target_link_libraries(deckz PRIVATE engine ui)
target_include_directories(deckz PRIVATE
    include
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}
)

add_library(ui src/ui/TerminalUI.cpp)
target_link_libraries(ui PUBLIC engine)
target_include_directories(ui PUBLIC include)

# --- Tests ---
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(engine_tests
    tests/test_card.cpp
    tests/test_deck.cpp
    tests/test_registry.cpp
    tests/test_hand.cpp
    tests/test_player.cpp
    tests/test_turn_engine.cpp
    tests/test_game_state.cpp
    tests/test_action_validator.cpp
    tests/test_effect.cpp
    tests/test_game.cpp
)

target_link_libraries(engine_tests PRIVATE engine GTest::gtest_main)
target_include_directories(engine_tests PRIVATE ${CMAKE_BINARY_DIR}/include)

include(GoogleTest)
gtest_discover_tests(engine_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
