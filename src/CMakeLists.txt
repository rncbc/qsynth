set(NAME qsynth)

include_directories (
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file (cmake_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

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

qt5_add_translation ( QM_FILES ${TRANSLATIONS} )
add_custom_target( translations ALL DEPENDS ${QM_FILES} )

qt5_wrap_ui (UI_SOURCES ${FORMS})
qt5_wrap_cpp (MOC_SOURCES ${HEADERS})
qt5_add_resources (QRC_SOURCES ${RESOURCES})

if (WIN32)
  set (RC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/win32/${NAME}.rc)
  set (RES_FILE ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.res.obj)
  find_program (WINDRES_EXECUTABLE NAMES windres mingw32-windres i686-mingw32-windres)
  if (MINGW)
    exec_program (${WINDRES_EXECUTABLE}
      ARGS "-i ${RC_FILE} -o ${RES_FILE} --include-dir=${CMAKE_CURRENT_SOURCE_DIR}/images")
    list (APPEND SOURCES ${RES_FILE})
  else ()
    list (APPEND SOURCES ${RC_FILE})
  endif ()
endif ()

if (APPLE)
  set (ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/images/${NAME}.icns)
  list (APPEND SOURCES ${ICON_FILE})
  set (MACOSX_BUNDLE_ICON_FILE ${NAME}.icns)
  set_source_files_properties (${ICON_FILE} PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
endif ()

add_executable (${NAME}
  ${UI_SOURCES}
  ${MOC_SOURCES}
  ${QRC_SOURCES}
  ${SOURCES}
)

set_target_properties (${NAME} PROPERTIES CXX_STANDARD 11)

if (WIN32)
  set_target_properties (${NAME} PROPERTIES WIN32_EXECUTABLE true)
endif ()

if (APPLE)
  set_target_properties (${NAME} PROPERTIES MACOSX_BUNDLE true)
endif ()

target_link_libraries (${NAME} PRIVATE Qt5::Widgets)

if (CONFIG_XUNIQUE)
  target_link_libraries (${NAME} PRIVATE Qt5::Network)
endif ()

if (CONFIG_FLUIDSYNTH)
  target_link_libraries (${NAME} PRIVATE ${FLUIDSYNTH_LIBRARIES})
endif ()

if (CONFIG_ROUND)
  target_link_libraries (${NAME} PRIVATE ${MATH_LIBRARY})
endif ()


if (UNIX AND NOT APPLE)
  install (TARGETS ${NAME} RUNTIME
     DESTINATION ${CMAKE_INSTALL_BINDIR})
  install (FILES ${QM_FILES}
     DESTINATION ${CMAKE_INSTALL_DATADIR}/${NAME}/translations)
  install (FILES ${NAME}.desktop
     DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
  install (FILES images/${NAME}.png
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps)
  install (FILES images/${NAME}.svg
     DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
  install (FILES appdata/${NAME}.appdata.xml
     DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
endif ()

if (WIN32)
   install (TARGETS ${NAME} RUNTIME
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
   install (FILES ${QM_FILES}
     DESTINATION ${CMAKE_INSTALL_DATADIR}/${NAME}/translations)
endif ()
