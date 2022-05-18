cmake_minimum_required(VERSION 3.5)

project(Client-Double-Exam LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt6 COMPONENTS Widgets LinguistTools REQUIRED)
#QT5_USE_MODULES(${EXECUTABLE_NAME} ... WebKit)

set(TS_FILES Client-Double-Exam_ru_RU.ts)

  add_executable(Client-Double-Exam WIN32
    FClient.h
    DEX_Client.cpp
    DEX_Cleint.h
    main.cpp
    tasks.cpp
    heart.cpp
    tasks.h
    heart.h
    heart.ui
    ${TS_FILES})


target_link_libraries(Client-Double-Exam PRIVATE wsock32 ws2_32)



target_link_libraries(Client-Double-Exam PRIVATE Qt6::Widgets)

#qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
