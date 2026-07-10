cmake_minimum_required(VERSION 3.25)

project(coolors_geode_mod VERSION 1.0.0 LANGUAGES CXX)

# Търси Geode SDK, което GitHub Action автоматично ще инсталира на сървъра
find_package(Geode REQUIRED)

# Настройка на мода
setup_geode_mod(coolors_geode_mod)

# Описва къде се намира основният C++ файл
target_sources(coolors_geode_mod PRIVATE src/main.cpp)
