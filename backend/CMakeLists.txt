cmake_minimum_required(VERSION 3.12...3.31)
project(backend CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DownloadUserver)

find_package(
    userver
    COMPONENTS core
               postgresql
    QUIET
)

if(NOT userver_FOUND)
    download_userver(TRY_DIR third_party/userver)
endif()

userver_setup_environment()

include_directories(src)

add_library(
    ${PROJECT_NAME}_objs OBJECT
    src/handlers/create_game_handler.cpp
    src/handlers/get_game_handler.cpp
    src/handlers/get_games_list_handler.cpp
    src/handlers/make_move_handler.cpp
    src/handlers/handler_utils.cpp
    src/handlers/resign_game_handler.cpp
    src/handlers/delete_game_handler.cpp
    src/handlers/get_legal_moves_handler.cpp

    src/repositories/game_repository.cpp

    src/services/game_service.cpp
    src/services/chess_service.cpp

    src/stockfish/stockfish_client.cpp
)

target_link_libraries(
    ${PROJECT_NAME}_objs
    PUBLIC userver::core
           userver::postgresql
)

target_include_directories(
    ${PROJECT_NAME}_objs
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)

include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
    message(STATUS "Set install prefix: $ENV{PREFIX}")
    file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
    set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(
    GLOB CONFIGS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml
    ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json
)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT ${PROJECT_NAME}
)

install(
    FILES ${CONFIGS_FILES}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    COMPONENT ${PROJECT_NAME}
)