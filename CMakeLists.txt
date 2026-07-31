cmake_minimum_required(VERSION 3.16)
project(LitePaste LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "D:/Qt_Soft/5.14.2/mingw73_64")
endif()

find_package(Qt5 5.14 REQUIRED COMPONENTS Widgets Concurrent Test)

set(LITEPASTE_CORE_SOURCES
    src/app/Application.h
    src/app/Application.cpp
    src/app/AppConfig.h
    src/app/AppConfig.cpp
    src/clipboard/ClipboardItem.h
    src/clipboard/ClipboardMonitor.h
    src/clipboard/ClipboardMonitor.cpp
    src/clipboard/ClipboardRepository.h
    src/clipboard/ClipboardRepository.cpp
    src/storage/HistoryStorage.h
    src/storage/FileHistoryStorage.h
    src/storage/FileHistoryStorage.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/ClipboardHistoryModel.h
    src/ui/ClipboardHistoryModel.cpp
    src/utils/TextNormalizer.h
    src/utils/TextNormalizer.cpp
    src/platform/PlatformAdapter.h
    src/paste/PasteStrategy.h
    src/paste/NormalPasteStrategy.cpp
    src/paste/PlainTextPasteStrategy.cpp
    src/paste/PasteController.h
    src/paste/PasteController.cpp
)

if(WIN32)
    list(APPEND LITEPASTE_CORE_SOURCES
        src/platform/windows/WindowsPlatformAdapter.h
        src/platform/windows/WindowsPlatformAdapter.cpp
    )
    # 必须在编译命令行层面指定，确保先于任何头文件（包括 Qt 头文件）生效，
    # 否则 RegisterHotKey 的 MOD_NOREPEAT 等 Windows 7+ 常量不可用。
    add_compile_definitions(_WIN32_WINNT=0x0601 WIN32_LEAN_AND_MEAN)
endif()

add_library(litepaste_core STATIC ${LITEPASTE_CORE_SOURCES})
target_include_directories(litepaste_core PUBLIC src)
target_link_libraries(litepaste_core PUBLIC Qt5::Widgets Qt5::Concurrent)

add_executable(LitePaste WIN32 src/main.cpp)
target_link_libraries(LitePaste PRIVATE litepaste_core)

enable_testing()
add_subdirectory(tests)
