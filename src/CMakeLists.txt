cmake_minimum_required(VERSION 3.10)

if (NOT ${CMAKE_VERSION} VERSION_LESS "3.0.0")
	cmake_policy(SET CMP0028 NEW)
endif()
if (NOT ${CMAKE_VERSION} VERSION_LESS "2.8.11")
	cmake_policy(SET CMP0020 NEW)
endif()

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX /wd4100 /wd4189")
else()
  add_definitions("-pedantic -Wall -Wextra -Werror -std=c++17")
endif()

if (APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
endif()

project(rclone-browser)
find_package(Qt6Core REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Network REQUIRED)

if (WIN32)
  find_package(Qt6WinExtras REQUIRED)
elseif(APPLE)
  find_package(Qt6MacExtras REQUIRED)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(UI
  main_window.ui
  remote_widget.ui
  transfer_dialog.ui
  export_dialog.ui
  progress_dialog.ui
  job_widget.ui
  mount_widget.ui
  stream_widget.ui
  preferences_dialog.ui
)

set(MOC
  main_window.h
  remote_widget.h
  transfer_dialog.h
  export_dialog.h
  progress_dialog.h
  job_widget.h
  mount_widget.h
  stream_widget.h
  preferences_dialog.h
  icon_cache.h
  list_of_job_options.h
  item_model.h
)

set(OTHER
  pch.h
  utils.h
  job_options.h
)

set(SOURCE
  pch.cpp
  main.cpp
  main_window.cpp
  remote_widget.cpp
  transfer_dialog.cpp
  export_dialog.cpp
  progress_dialog.cpp
  job_widget.cpp
  mount_widget.cpp
  stream_widget.cpp
  preferences_dialog.cpp
  icon_cache.cpp
  item_model.cpp
  utils.cpp
  job_options.cpp
  list_of_job_options.cpp
)

if(WIN32)
  set(OTHER ${OTHER} resources.rc)
elseif(APPLE)
  set(OTHER ${OTHER} osx_helper.h)
  set(SOURCE ${SOURCE} osx_helper.mm)
endif()

set(QRC resources.qrc)

add_definitions(-DRCLONE_BROWSER_VERSION="${RCLONE_BROWSER_VERSION}")

qt_wrap_ui(UI_OUT ${UI})
qt_wrap_cpp(MOC_OUT ${MOC})
qt_add_resources(QRC_OUT ${QRC} OPTIONS "-no-compress")

source_group("" FILES ${SOURCE} ${MOC} ${UI} ${QRC} ${OTHER})
source_group("Generated" FILES ${MOC_OUT} ${UI_OUT} ${MOC_OUT} ${QRC_OUT})

MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

ADD_MSVC_PRECOMPILED_HEADER(pch.h pch.cpp SOURCE)
ADD_MSVC_PRECOMPILED_HEADER(pch.h pch.cpp MOC_OUT)

if(WIN32)
  add_executable(RcloneBrowser WIN32 ${SOURCE} ${BACKEND} ${OTHER} ${MOC} ${MOC_OUT} ${UI_OUT} ${MOC_OUT} ${QRC_OUT})
  target_link_libraries(RcloneBrowser Qt6::Widgets Qt6::Network Qt6::WinExtras)
elseif(APPLE)
  set_source_files_properties(icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  add_executable(rclone-browser MACOSX_BUNDLE ${SOURCE} ${BACKEND} ${OTHER} ${MOC} ${MOC_OUT} ${UI_OUT} ${MOC_OUT} ${QRC_OUT} icon.icns)
  target_link_libraries(rclone-browser Qt6::Widgets Qt6::Network Qt6::MacExtras ${COCOA_LIB})
  set_target_properties(rclone-browser PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")

else()
  add_executable(rclone-browser ${SOURCE} ${BACKEND} ${OTHER} ${MOC} ${MOC_OUT} ${UI_OUT} ${MOC_OUT} ${QRC_OUT})
  target_link_libraries(rclone-browser Qt6::Widgets Qt6::Network)

  install(TARGETS rclone-browser RUNTIME DESTINATION bin)
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser.svg" DESTINATION "share/icons/hicolor/scalable/apps" RENAME "rclone-browser.svg")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser-32x32.png" DESTINATION "share/icons/hicolor/32x32/apps" RENAME "rclone-browser.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser-64x64.png" DESTINATION "share/icons/hicolor/64x64/apps" RENAME "rclone-browser.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser-128x128.png" DESTINATION "share/icons/hicolor/128x128/apps" RENAME "rclone-browser.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser-256x256.png" DESTINATION "share/icons/hicolor/256x256/apps" RENAME "rclone-browser.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser-512x512.png" DESTINATION "share/icons/hicolor/512x512/apps" RENAME "rclone-browser.png")
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../assets/rclone-browser.desktop" DESTINATION "share/applications")
endif()
