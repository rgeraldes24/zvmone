# zvmone: Fast Zond Virtual Machine implementation
# Copyright 2022 The evmone Authors.
# SPDX-License-Identifier: Apache-2.0

hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(zvmone-statetestutils STATIC)
add_library(zvmone::statetestutils ALIAS zvmone-statetestutils)
target_compile_features(zvmone-statetestutils PUBLIC cxx_std_20)
target_link_libraries(zvmone-statetestutils PRIVATE zvmone::state nlohmann_json::nlohmann_json)
target_include_directories(zvmone-statetestutils PRIVATE ${zvmone_private_include_dir})
target_sources(
    zvmone-statetestutils PRIVATE
    statetest.hpp
    statetest_loader.cpp
    statetest_logs_hash.cpp
)

add_executable(zvmone-statetest)
target_link_libraries(zvmone-statetest PRIVATE zvmone::statetestutils zvmone GTest::gtest)
target_include_directories(zvmone-statetest PRIVATE ${zvmone_private_include_dir})
target_sources(
    zvmone-statetest PRIVATE
    statetest.cpp
    statetest_runner.cpp
)
