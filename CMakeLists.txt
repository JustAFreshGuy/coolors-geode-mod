cmake_minimum_required(VERSION 3.25)

# Конфигурира Geode проекта
include(FetchContent)
FetchContent_Declare(
    GeodeFetch
    GIT_REPOSITORY https://github.com
    GIT_TAG main
)
FetchContent_MakeAvailable(GeodeFetch)

project(coolors_geode_mod VERSION 1.0.0 LANGUAGES CXX)

# Добавяне на вашия мод
setup_geode_mod(coolors_geode_mod)

# Казва на компилатора къде се намира C++ кода
target_sources(coolors_geode_mod PRIVATE src/main.cpp)
