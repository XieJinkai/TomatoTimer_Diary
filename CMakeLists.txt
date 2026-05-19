cmake_minimum_required(VERSION 3.16)
project(TomatoTimerQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Sql)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test QUIET)
find_package(Python3 COMPONENTS Interpreter QUIET)

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
  qt/pages/AccountingPage.h qt/pages/AccountingPage.cpp
  qt/pages/ImageToolsPage.h qt/pages/ImageToolsPage.cpp
  qt/pages/SettingsSyncPage.h qt/pages/SettingsSyncPage.cpp
  qt/services/AccountingStore.h qt/services/AccountingStore.cpp
  qt/services/AccountingStatsService.h qt/services/AccountingStatsService.cpp
  qt/services/Session.h qt/services/Session.cpp
  qt/services/DiaryStore.h qt/services/DiaryStore.cpp
  qt/services/DiaryImageService.h qt/services/DiaryImageService.cpp
  qt/services/DiaryDocumentCodec.h qt/services/DiaryDocumentCodec.cpp
  qt/services/DataStore.h qt/services/DataStore.cpp
  qt/services/CloudSyncService.h qt/services/CloudSyncService.cpp
  qt/services/FocusRecordService.h qt/services/FocusRecordService.cpp
  qt/services/StatsService.h qt/services/StatsService.cpp
  qt/services/SpeechToText.h qt/services/SpeechToText.cpp
  qt/services/OcrService.h qt/services/OcrService.cpp
  qt/services/PaddleInferOcr.h qt/services/PaddleInferOcr.cpp
  qt/ui/Theme.h qt/ui/Theme.cpp
  qt/ui/ImageView.h qt/ui/ImageView.cpp
  qt/ui/charts/BarChartWidget.h qt/ui/charts/BarChartWidget.cpp
  qt/ui/charts/PieChartWidget.h qt/ui/charts/PieChartWidget.cpp
)

target_include_directories(tomato_timer PRIVATE qt qt/pages qt/services qt/windows qt/ui qt/ui/charts ./include ./include/opencv2)
target_link_libraries(tomato_timer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

add_executable(local_api_server
  qt/server/LocalApiServer.h qt/server/LocalApiServer.cpp
  qt/server/LocalApiServerMain.cpp
)
target_link_libraries(local_api_server PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Sql)


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

set(OCR_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/include/bin/ocr_paddle.py")
set(OCR_PYTHON_EXE "")
if(Python3_Interpreter_FOUND)
  set(OCR_PYTHON_EXE "${Python3_EXECUTABLE}")
endif()
if(WIN32 AND OCR_PYTHON_EXE STREQUAL "")
  set(OCR_WIN_PYTHON "C:/Users/19231/AppData/Local/Programs/Python/Python313/python.exe")
  if(EXISTS "${OCR_WIN_PYTHON}")
    set(OCR_PYTHON_EXE "${OCR_WIN_PYTHON}")
  endif()
endif()
set(OCR_PYTHON_SITE_PACKAGES "")
if(WIN32)
  set(OCR_WIN_USER_SITE_PACKAGES "C:/Users/19231/AppData/Roaming/Python/Python313/site-packages")
  set(OCR_WIN_SITE_PACKAGES "C:/Users/19231/AppData/Local/Programs/Python/Python313/Lib/site-packages")
  if(EXISTS "${OCR_WIN_USER_SITE_PACKAGES}")
    set(OCR_PYTHON_SITE_PACKAGES "${OCR_WIN_USER_SITE_PACKAGES}")
  elseif(EXISTS "${OCR_WIN_SITE_PACKAGES}")
    set(OCR_PYTHON_SITE_PACKAGES "${OCR_WIN_SITE_PACKAGES}")
  endif()
endif()
if(EXISTS "${OCR_SCRIPT_PATH}")
  file(TO_CMAKE_PATH "${OCR_SCRIPT_PATH}" OCR_SCRIPT_PATH)
  target_compile_definitions(tomato_timer PRIVATE OCR_DEFAULT_PADDLE_SCRIPT="${OCR_SCRIPT_PATH}")
endif()
if(NOT OCR_PYTHON_EXE STREQUAL "")
  file(TO_CMAKE_PATH "${OCR_PYTHON_EXE}" OCR_PYTHON_EXE)
  target_compile_definitions(tomato_timer PRIVATE OCR_DEFAULT_PYTHON_EXE="${OCR_PYTHON_EXE}")
endif()
if(NOT OCR_PYTHON_SITE_PACKAGES STREQUAL "")
  file(TO_CMAKE_PATH "${OCR_PYTHON_SITE_PACKAGES}" OCR_PYTHON_SITE_PACKAGES)
  target_compile_definitions(tomato_timer PRIVATE OCR_DEFAULT_PYTHON_SITE_PACKAGES="${OCR_PYTHON_SITE_PACKAGES}")
endif()

# Link local OpenCV world library from ./include/lib without relying on system OpenCV
set(OPENCV_LOCAL_LIB_DIR "${CMAKE_SOURCE_DIR}/include/lib")
if(MSVC)
  set(OPENCV_WORLD_RELEASE "${OPENCV_LOCAL_LIB_DIR}/opencv_world4120.lib")
  set(OPENCV_WORLD_DEBUG   "${OPENCV_LOCAL_LIB_DIR}/opencv_world4120d.lib")
  target_link_libraries(tomato_timer PRIVATE
    $<$<CONFIG:Debug>:${OPENCV_WORLD_DEBUG}>
    $<$<CONFIG:Release>:${OPENCV_WORLD_RELEASE}>
  )
  target_compile_definitions(tomato_timer PRIVATE WITH_OPENCV=1)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Test AND EXISTS "${CMAKE_SOURCE_DIR}/qt/tests/AccountingPageTests.cpp" AND EXISTS "${CMAKE_SOURCE_DIR}/qt/pages/AccountingPage.cpp" AND EXISTS "${CMAKE_SOURCE_DIR}/qt/services/AccountingStore.cpp")
  enable_testing()
  add_executable(accounting_page_tests
    qt/tests/AccountingPageTests.cpp
    qt/pages/AccountingPage.h qt/pages/AccountingPage.cpp
    qt/services/AccountingStore.h qt/services/AccountingStore.cpp
    qt/services/DataStore.h qt/services/DataStore.cpp
    qt/services/Session.h qt/services/Session.cpp
  )
  target_include_directories(accounting_page_tests PRIVATE qt qt/pages qt/services qt/windows qt/ui qt/ui/charts ./include ./include/opencv2)
  target_link_libraries(accounting_page_tests PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
  add_test(NAME accounting_page_tests COMMAND accounting_page_tests)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Test AND EXISTS "${CMAKE_SOURCE_DIR}/qt/tests/CloudSyncServiceTests.cpp")
  enable_testing()
  add_executable(cloud_sync_service_tests
    qt/tests/CloudSyncServiceTests.cpp
    qt/services/CloudSyncService.h qt/services/CloudSyncService.cpp
    qt/services/DataStore.h qt/services/DataStore.cpp
  )
  target_include_directories(cloud_sync_service_tests PRIVATE qt qt/services)
  target_link_libraries(cloud_sync_service_tests PRIVATE Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Test)
  add_test(NAME cloud_sync_service_tests COMMAND cloud_sync_service_tests)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Test AND EXISTS "${CMAKE_SOURCE_DIR}/qt/tests/DiaryImageServiceTests.cpp")
  enable_testing()
  add_executable(diary_image_service_tests
    qt/tests/DiaryImageServiceTests.cpp
    qt/services/DiaryImageService.h qt/services/DiaryImageService.cpp
    qt/services/DataStore.h qt/services/DataStore.cpp
  )
  target_include_directories(diary_image_service_tests PRIVATE qt qt/services)
  target_link_libraries(diary_image_service_tests PRIVATE Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Test)
  add_test(NAME diary_image_service_tests COMMAND diary_image_service_tests)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Test AND EXISTS "${CMAKE_SOURCE_DIR}/qt/tests/DiaryDocumentCodecTests.cpp")
  enable_testing()
  add_executable(diary_document_codec_tests
    qt/tests/DiaryDocumentCodecTests.cpp
    qt/services/DiaryDocumentCodec.h qt/services/DiaryDocumentCodec.cpp
    qt/services/DiaryImageService.h qt/services/DiaryImageService.cpp
    qt/services/DataStore.h qt/services/DataStore.cpp
  )
  target_include_directories(diary_document_codec_tests PRIVATE qt qt/services)
  target_link_libraries(diary_document_codec_tests PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
  add_test(NAME diary_document_codec_tests COMMAND diary_document_codec_tests)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Test AND EXISTS "${CMAKE_SOURCE_DIR}/qt/tests/LocalApiServerTests.cpp")
  enable_testing()
  add_executable(local_api_server_tests
    qt/tests/LocalApiServerTests.cpp
    qt/server/LocalApiServer.h qt/server/LocalApiServer.cpp
  )
  target_include_directories(local_api_server_tests PRIVATE qt qt/server)
  target_link_libraries(local_api_server_tests PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Test)
  add_test(NAME local_api_server_tests COMMAND local_api_server_tests)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Test AND EXISTS "${CMAKE_SOURCE_DIR}/qt/tests/LoginRegisterPageApiTests.cpp")
  enable_testing()
  add_executable(login_register_page_api_tests
    qt/tests/LoginRegisterPageApiTests.cpp
    qt/pages/LoginRegisterPage.h qt/pages/LoginRegisterPage.cpp
    qt/services/Session.h qt/services/Session.cpp
  )
  target_include_directories(login_register_page_api_tests PRIVATE qt qt/pages qt/services)
  target_link_libraries(login_register_page_api_tests PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Test)
  add_test(NAME login_register_page_api_tests COMMAND login_register_page_api_tests)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Test AND EXISTS "${CMAKE_SOURCE_DIR}/qt/tests/LoginWindowFlowTests.cpp")
  enable_testing()
  add_executable(login_window_flow_tests
    qt/tests/LoginWindowFlowTests.cpp
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
    qt/pages/AccountingPage.h qt/pages/AccountingPage.cpp
    qt/pages/ImageToolsPage.h qt/pages/ImageToolsPage.cpp
    qt/pages/SettingsSyncPage.h qt/pages/SettingsSyncPage.cpp
    qt/services/AccountingStore.h qt/services/AccountingStore.cpp
    qt/services/AccountingStatsService.h qt/services/AccountingStatsService.cpp
    qt/services/Session.h qt/services/Session.cpp
    qt/services/DiaryStore.h qt/services/DiaryStore.cpp
    qt/services/DiaryImageService.h qt/services/DiaryImageService.cpp
    qt/services/DiaryDocumentCodec.h qt/services/DiaryDocumentCodec.cpp
    qt/services/DataStore.h qt/services/DataStore.cpp
    qt/services/CloudSyncService.h qt/services/CloudSyncService.cpp
    qt/services/FocusRecordService.h qt/services/FocusRecordService.cpp
    qt/services/StatsService.h qt/services/StatsService.cpp
    qt/services/SpeechToText.h qt/services/SpeechToText.cpp
    qt/services/OcrService.h qt/services/OcrService.cpp
    qt/services/PaddleInferOcr.h qt/services/PaddleInferOcr.cpp
    qt/ui/Theme.h qt/ui/Theme.cpp
    qt/ui/ImageView.h qt/ui/ImageView.cpp
    qt/ui/charts/BarChartWidget.h qt/ui/charts/BarChartWidget.cpp
    qt/ui/charts/PieChartWidget.h qt/ui/charts/PieChartWidget.cpp
  )
  target_include_directories(login_window_flow_tests PRIVATE qt qt/pages qt/services qt/windows qt/ui qt/ui/charts ./include ./include/opencv2)
  target_link_libraries(login_window_flow_tests PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Test)
  if(WIN32)
    target_link_libraries(login_window_flow_tests PRIVATE sapi)
  endif()
  add_test(NAME login_window_flow_tests COMMAND login_window_flow_tests)
endif()

if(TARGET Qt${QT_VERSION_MAJOR}::Test AND EXISTS "${CMAKE_SOURCE_DIR}/qt/tests/FocusRecordServiceTests.cpp")
  enable_testing()
  add_executable(focus_record_service_tests
    qt/tests/FocusRecordServiceTests.cpp
    qt/services/FocusRecordService.h qt/services/FocusRecordService.cpp
  )
  target_include_directories(focus_record_service_tests PRIVATE qt qt/services)
  target_link_libraries(focus_record_service_tests PRIVATE Qt${QT_VERSION_MAJOR}::Test)
  add_test(NAME focus_record_service_tests COMMAND focus_record_service_tests)
endif()
