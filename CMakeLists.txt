cmake_minimum_required(VERSION 3.14)

project(pjsip-tests-qt LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)
file(GLOB MOC_HDRS ${PROJECT_SOURCE_DIR}/*.hpp )
include_directories(${PROJECT_SOURCE_DIR})

include (FindPkgConfig)
if (PKG_CONFIG_FOUND) # true if pkg-config works on the system
  pkg_check_modules(PJSIP REQUIRED libpjproject)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${MOC_HDRS})

include_directories(${PJSIP_STATIC_INCLUDE_DIRS})
link_directories(${PJSIP_STATIC_LIBRARY_DIRS})

target_link_libraries(pjsip-tests-qt Qt${QT_VERSION_MAJOR}::Core ${PJSIP_STATIC_LIBRARIES})

install(TARGETS pjsip-tests-qt
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
