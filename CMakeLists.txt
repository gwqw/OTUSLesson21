cmake_minimum_required(VERSION 3.2)

# build number for travis
if(DEFINED ENV{TRAVIS_BUILD_NUMBER})
    project(async VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else()
    project(async VERSION 0.0.1)
endif()

set(USE_TEST ON)

# boost dependensies
if (USE_TEST)
    find_package(Boost COMPONENTS unit_test_framework REQUIRED)
endif()
find_package(Threads REQUIRED)

# source
set(SOURCE bulk.cpp bulk.h
    command_reader.cpp command_reader.h
    command_handler.cpp command_handler.h
    command_processor.cpp command_processor.h
    command.h
    ts_cont.h
    thread_pool.cpp thread_pool.h)
set(LIB_SOURCE async.cpp async.h ${SOURCE})
set(EXE_SOURCE main.cpp ${SOURCE})
if (USE_TEST)
    set(TEST_SOURCE test_async.cpp ${SOURCE})
endif()

# targets and libraries
set(LIB_NAME async)
set(EXE_NAME async_cli)
if (USE_TEST)
    set(TEST_NAME test_async)
endif()
add_library(${LIB_NAME} SHARED ${LIB_SOURCE})
add_executable(${EXE_NAME} ${EXE_SOURCE})
if (USE_TEST)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
endif()

# compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMP_OPTIONS "-pthread;-Wall;-Wextra")
  if(CMAKE_BUILD_TYPE MATCHES Debug)
      set(CMP_OPTIONS ${CMP_OPTIONS}";-g")
      set(CMP_OPTIONS ${CMP_OPTIONS}";-fsanitize=thread")
      message("debug")
  endif()
else()
  set(CMP_OPTIONS "/W4")
endif()

# target properties
set_target_properties(${LIB_NAME} ${EXE_NAME} ${TEST_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS ${CMP_OPTIONS}
    LINK_OPTIONS "-fsanitize=thread"
)

# add boost headers for test
if (USE_TEST)
    if(UNIX)
        set_target_properties(${TEST_NAME} PROPERTIES
            COMPILE_DEFINITIONS BOOST_TEST_DYN_LINK
        )
    endif()
    target_include_directories(${TEST_NAME}
            PRIVATE ${Boost_INCLUDE_DIR}
    )
endif()

# target linking
target_link_libraries(${EXE_NAME} ${LIB_NAME})
target_link_libraries(${LIB_NAME} Threads::Threads)
if (USE_TEST)
    target_link_libraries(${TEST_NAME}
        ${Boost_LIBRARIES}
    )
    target_link_libraries(${TEST_NAME}
        Threads::Threads
    )
endif()

# installation
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION lib)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT gwrass@mail.com)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)

# tests
if (USE_TEST)
    enable_testing()
    add_test(${TEST_NAME} ${TEST_NAME})
endif()
