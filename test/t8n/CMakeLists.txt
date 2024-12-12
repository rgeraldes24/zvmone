# zvmone: Fast Zond Virtual Machine implementation
# Copyright 2022 The evmone Authors.
# SPDX-License-Identifier: Apache-2.0

include(CableBuildInfo)
cable_add_buildinfo_library(PROJECT_NAME zvmone)

hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(zvmone-t8n)
target_link_libraries(zvmone-t8n PRIVATE zvmone::statetestutils nlohmann_json::nlohmann_json)
target_link_libraries(zvmone-t8n PRIVATE zvmc::zvmc zvmone zvmone-buildinfo)
target_sources(zvmone-t8n PRIVATE t8n.cpp)
