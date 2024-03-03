# project (qsynth)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
  file (REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
endif ()
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set (HEADERS
  qsynth.h
  qsynthKnob.h
  qsynthMeter.h
  qsynthSystemTray.h
  qsynthTabBar.h
  qsynthAboutForm.h
  qsynthChannelsForm.h
  qsynthMainForm.h
  qsynthMessagesForm.h
  qsynthOptionsForm.h
  qsynthPaletteForm.h
  qsynthPresetForm.h
  qsynthSetupForm.h
)

set (SOURCES
  qsynth.cpp
  qsynthEngine.cpp
  qsynthChannels.cpp
  qsynthKnob.cpp
  qsynthMeter.cpp
  qsynthSetup.cpp
  qsynthOptions.cpp
  qsynthSystemTray.cpp
  qsynthTabBar.cpp
  qsynthAboutForm.cpp
  qsynthChannelsForm.cpp
  qsynthMainForm.cpp
  qsynthMessagesForm.cpp
  qsynthOptionsForm.cpp
  qsynthPaletteForm.cpp
  qsynthPresetForm.cpp
  qsynthSetupForm.cpp
  qsynthDialClassicStyle.cpp
  qsynthDialPeppinoStyle.cpp
  qsynthDialVokiStyle.cpp
  qsynthDialSkulptureStyle.cpp
)

set (FORMS
  qsynthAboutForm.ui
  qsynthChannelsForm.ui
  qsynthMainForm.ui
  qsynthMessagesForm.ui
  qsynthOptionsForm.ui
  qsynthPaletteForm.ui
  qsynthPresetForm.ui
  qsynthSetupForm.ui
)

set (RESOURCES
  qsynth.qrc
)

set ( TRANSLATIONS
  translations/qsynth_cs.ts
  translations/qsynth_de.ts
  translations/qsynth_es.ts
  translations/qsynth_fr.ts
  translations/qsynth_ru.ts
  translations/qsynth_sr.ts
)

add_custom_target (update_translations
  COMMAND Qt${QT_VERSION_MAJOR}::lupdate -recursive ${CMAKE_CURRENT_SOURCE_DIR} -ts ${TRANSLATIONS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Updating translations..."
)

if (QT_VERSION VERSION_LESS 5.15.0)
  qt5_add_translation (QM_FILES ${TRANSLATIONS})
else()
  qt_add_translation (QM_FILES ${TRANSLATIONS})
endif()

add_custom_target (translations ALL DEPENDS ${QM_FILES})

if (WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/win32/setup.nsi.in
                 ${CMAKE_CURRENT_BINARY_DIR}/setup.nsi IMMEDIATE @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/win32/${PROJECT_NAME}.rc.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc IMMEDIATE @ONLY)
  set (RC_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc)
  set (RES_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.res.obj)
  if (MINGW)
    get_filename_component (compiler_location "${CMAKE_CXX_COMPILER}" DIRECTORY)
    find_program (WINDRES_EXECUTABLE NAMES windres mingw32-windres i686-mingw32-windres PATHS ${compiler_location} NO_DEFAULT_PATH)
    execute_process (COMMAND ${WINDRES_EXECUTABLE} -i ${RC_FILE} -o ${RES_FILE} --include-dir=${CMAKE_CURRENT_SOURCE_DIR}/images/
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    list (APPEND SOURCES ${RES_FILE})
  else ()
    list (APPEND SOURCES ${RC_FILE})
  endif ()
endif ()

if (APPLE)
  set (ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/images/${PROJECT_NAME}.icns)
  list (APPEND SOURCES ${ICON_FILE} ${QM_FILES})
  set (MACOSX_BUNDLE_ICON_FILE ${PROJECT_NAME}.icns)
  set_source_files_properties (${ICON_FILE} PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
  set_source_files_properties (${QM_FILES} PROPERTIES
    MACOSX_PACKAGE_LOCATION translations)
  include(TranslationUtils)
  add_qt_translations(QT_TRANSLATIONS cs de es fr ru sr)
  set_source_files_properties (${QT_TRANSLATIONS} PROPERTIES
    MACOSX_PACKAGE_LOCATION translations)
  list (APPEND SOURCES ${QT_TRANSLATIONS})
endif ()


add_executable (${PROJECT_NAME}
  ${HEADERS}
  ${SOURCES}
  ${FORMS}
  ${RESOURCES}
)

# Add some debugger flags.
if (CONFIG_DEBUG AND UNIX AND NOT APPLE)
  set (CONFIG_DEBUG_OPTIONS -g -fsanitize=address -fno-omit-frame-pointer)
  target_compile_options (${PROJECT_NAME} PRIVATE ${CONFIG_DEBUG_OPTIONS})
  target_link_options (${PROJECT_NAME} PRIVATE ${CONFIG_DEBUG_OPTIONS})
endif ()

set_target_properties (${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

if (WIN32)
  set_target_properties (${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE true)
endif ()

if (APPLE)
  set_target_properties (${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE true
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.rncbc.${PROJECT_NAME}"
    MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
    MACOSX_BUNDLE_DISPLAY_NAME "${PROJECT_NAME}"
    MACOSX_BUNDLE_INFO_STRING "${PROJECT_DESCRIPTION}"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "Copyright © 2003-2024, Rui Nuno Capela. All rights reserved.")
endif ()

target_link_libraries (${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Svg)

if (CONFIG_XUNIQUE)
  target_link_libraries (${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Network)
endif ()

if (CONFIG_FLUIDSYNTH)
  target_link_libraries (${PROJECT_NAME} PRIVATE PkgConfig::FLUIDSYNTH)
endif ()

if (CONFIG_ROUND)
  target_link_libraries (${PROJECT_NAME} PRIVATE ${MATH_LIBRARY})
endif ()

if (CONFIG_PIPEWIRE)
  target_link_libraries (${PROJECT_NAME} PRIVATE PkgConfig::PIPEWIRE)
endif ()


if (UNIX AND NOT APPLE)
  install (TARGETS ${PROJECT_NAME} RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR})
  install (FILES ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)
  install (FILES images/${PROJECT_NAME}.png
    RENAME org.rncbc.${PROJECT_NAME}.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps)
  install (FILES images/${PROJECT_NAME}.svg
    RENAME org.rncbc.${PROJECT_NAME}.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
  install (FILES appdata/org.rncbc.${PROJECT_NAME}.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
  install (FILES appdata/org.rncbc.${PROJECT_NAME}.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
  install (FILES man1/${PROJECT_NAME}.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
  install (FILES man1/${PROJECT_NAME}.fr.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/fr/man1 RENAME ${PROJECT_NAME}.1)
  install (FILES palette/KXStudio.conf palette/Wonton\ Soup.conf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/palette)
endif ()

if (WIN32)
   install (TARGETS ${PROJECT_NAME} RUNTIME
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
   install (FILES ${QM_FILES}
     DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)
endif ()

include(DeploymentUtils)
