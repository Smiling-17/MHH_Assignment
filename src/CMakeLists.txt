cmake_minimum_required(VERSION 3.10)
project(PetriNetSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# 1. Tìm các thư viện cần thiết
# ============================================================

# TinyXML2 - XML Parser
find_package(tinyxml2 CONFIG QUIET)
if(NOT tinyxml2_FOUND)
    message(STATUS "tinyxml2 not found via find_package, using local source")
    set(USE_LOCAL_TINYXML2 TRUE)
endif()

# GLPK - ILP Solver
find_library(GLPK_LIBRARY NAMES glpk)
find_path(GLPK_INCLUDE_DIR NAMES glpk.h)

if(GLPK_LIBRARY AND GLPK_INCLUDE_DIR)
    message(STATUS "Found GLPK: ${GLPK_LIBRARY}")
    set(GLPK_FOUND TRUE)
else()
    message(WARNING "GLPK not found! ILP features will be disabled.")
    set(GLPK_FOUND FALSE)
endif()

# ============================================================
# 2. Lấy file mã nguồn BuDDy
# ============================================================
file(GLOB BUDDY_SOURCES "buddy/*.c")
list(APPEND BUDDY_SOURCES "buddy/cppext.cxx")

# ============================================================
# 3. Thêm đường dẫn include
# ============================================================
include_directories(.)
include_directories(buddy)

if(GLPK_FOUND)
    include_directories(${GLPK_INCLUDE_DIR})
endif()

# ============================================================
# 4. Khai báo file nguồn cho main executable
# ============================================================
set(MAIN_SOURCES
    main.cpp
    parser.cpp
    reachability.cpp
    bdd.cpp
    ${BUDDY_SOURCES}
)

# Thêm tinyxml2 local nếu cần
if(USE_LOCAL_TINYXML2)
    list(APPEND MAIN_SOURCES tinyxml2.cpp)
endif()

# Thêm ILP nếu có GLPK
if(GLPK_FOUND)
    list(APPEND MAIN_SOURCES ilp.cpp)
    add_definitions(-DHAS_GLPK)
endif()

# ============================================================
# 5. Tạo executable chính
# ============================================================
add_executable(petri_solver ${MAIN_SOURCES})

# Link thư viện
if(NOT USE_LOCAL_TINYXML2)
    target_link_libraries(petri_solver PRIVATE tinyxml2::tinyxml2)
endif()

if(GLPK_FOUND)
    target_link_libraries(petri_solver PRIVATE ${GLPK_LIBRARY})
endif()

# ============================================================
# 6. Tạo các test executables
# ============================================================

# Test Parser
add_executable(test_parser
    ../testcase/test_parser.cpp
    parser.cpp
    ${BUDDY_SOURCES}
)
if(USE_LOCAL_TINYXML2)
    target_sources(test_parser PRIVATE tinyxml2.cpp)
else()
    target_link_libraries(test_parser PRIVATE tinyxml2::tinyxml2)
endif()

# Test Reachability
add_executable(test_reach
    ../testcase/test_reach.cpp
    reachability.cpp
)

# Test BDD
add_executable(test_bdd
    ../testcase/test_bdd.cpp
    bdd.cpp
    ${BUDDY_SOURCES}
)

# Test ILP (chỉ khi có GLPK)
if(GLPK_FOUND)
    add_executable(test_ilp
        ../testcase/test_ilp.cpp
        ilp.cpp
        bdd.cpp
        ${BUDDY_SOURCES}
    )
    target_link_libraries(test_ilp PRIVATE ${GLPK_LIBRARY})
endif()

# ============================================================
# 7. Cài đặt output directory
# ============================================================
set_target_properties(petri_solver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin"
)

# ============================================================
# 8. Custom target để chạy test
# ============================================================
add_custom_target(run_tests
    COMMAND test_parser
    COMMAND test_reach
    COMMAND test_bdd
    DEPENDS test_parser test_reach test_bdd
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running all tests..."
)

message(STATUS "")
message(STATUS "=== Build Configuration ===")
message(STATUS "TinyXML2: ${tinyxml2_FOUND} (local: ${USE_LOCAL_TINYXML2})")
message(STATUS "GLPK: ${GLPK_FOUND}")
message(STATUS "===========================")
