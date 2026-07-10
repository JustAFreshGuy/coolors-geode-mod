cmake_minimum_required(VERSION 3.25)

project(coolors_geode_mod VERSION 1.0.0 LANGUAGES CXX)

# Търсене на пакета с малки букви, както се изисква в Geode v3
find_package(geode REQUIRED)

setup_geode_mod(coolors_geode_mod)

target_sources(coolors_geode_mod PRIVATE src/main.cpp)
