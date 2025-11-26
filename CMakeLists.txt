cmake_minimum_required(VERSION 3.16)
project(TomatoTimerQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Optional modules (guarded)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Multimedia QUIET)
set(HAVE_QT_MULTIMEDIA ${Qt${QT_VERSION_MAJOR}Multimedia_FOUND})

# Optional OpenCV (guarded)
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
  message(STATUS "OpenCV found: ${OpenCV_VERSION}")
  add_definitions(-DWITH_OPENCV)
endif()

add_executable(tomato_timer
  qt/main.cpp
  qt/LoginWindow.h qt/LoginWindow.cpp
  qt/MainWindow.h qt/MainWindow.cpp
  qt/windows/CalendarWindow.h qt/windows/CalendarWindow.cpp
  qt/windows/DiaryWindow.h qt/windows/DiaryWindow.cpp
  qt/pages/LoginRegisterPage.h qt/pages/LoginRegisterPage.cpp
  qt/pages/PomodoroPage.h qt/pages/PomodoroPage.cpp
  qt/pages/StopwatchPage.h qt/pages/StopwatchPage.cpp
  qt/pages/DiaryPage.h qt/pages/DiaryPage.cpp
  qt/pages/DiaryTabPage.h qt/pages/DiaryTabPage.cpp
  qt/pages/StatsPage.h qt/pages/StatsPage.cpp
  qt/pages/ImageToolsPage.h qt/pages/ImageToolsPage.cpp
  qt/pages/SettingsSyncPage.h qt/pages/SettingsSyncPage.cpp
  qt/services/Session.h qt/services/Session.cpp
  qt/services/DiaryStore.h qt/services/DiaryStore.cpp
  qt/services/DataStore.h qt/services/DataStore.cpp
  qt/services/StatsService.h qt/services/StatsService.cpp
  qt/services/SpeechToText.h qt/services/SpeechToText.cpp
  qt/ui/Theme.h qt/ui/Theme.cpp
)

target_include_directories(tomato_timer PRIVATE qt qt/pages qt/services qt/windows qt/ui)
target_link_libraries(tomato_timer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(HAVE_QT_MULTIMEDIA)
  target_link_libraries(tomato_timer PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
  target_compile_definitions(tomato_timer PRIVATE HAVE_QT_MULTIMEDIA=1)
endif()

if(OpenCV_FOUND)
  target_link_libraries(tomato_timer PRIVATE ${OpenCV_LIBS})
endif()

if(WIN32)
  target_link_libraries(tomato_timer PRIVATE sapi)
endif()
