include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${QT_INCLUDES}
    ${FLUIDSYNTH_INCLUDEDIR}
)

link_directories (
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_LIBRARY_DIR}
)

configure_file ( cmakeconfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

set ( ac_prefix ${CMAKE_INSTALL_PREFIX} )
configure_file ( qsynth.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/qsynth.desktop )

set ( HEADERS
    qsynthKnob.h
    qsynthMeter.h
    qsynthSystemTray.h
    qsynthTabBar.h
    qsynthAboutForm.h
    qsynthChannelsForm.h
    qsynthMainForm.h
    qsynthMessagesForm.h
    qsynthOptionsForm.h
    qsynthPresetForm.h
    qsynthSetupForm.h
)

set ( SOURCES
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
    qsynthPresetForm.cpp
    qsynthSetupForm.cpp
    qsynthDialClassicStyle.cpp
    qsynthDialPeppinoStyle.cpp
    qsynthDialVokiStyle.cpp
    qsynthDialSkulptureStyle.cpp
)

set ( FORMS
    qsynthAboutForm.ui
    qsynthChannelsForm.ui
    qsynthMainForm.ui
    qsynthMessagesForm.ui
    qsynthOptionsForm.ui
    qsynthPresetForm.ui
    qsynthSetupForm.ui
)

set ( RESOURCES
    qsynth.qrc
)

qt5_wrap_ui ( UI_SOURCES ${FORMS} )
qt5_wrap_cpp ( MOC_SOURCES ${HEADERS} )
qt5_add_resources ( QRC_SOURCES ${RESOURCES} )

if (WIN32)
    set ( RC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/win32/qsynth.rc )
    set ( RES_FILE ${CMAKE_CURRENT_BINARY_DIR}/qsynth.res.obj )
    find_program ( WINDRES_EXECUTABLE NAMES windres mingw32-windres i686-mingw32-windres)
    if (MINGW)
        exec_program( ${WINDRES_EXECUTABLE}
            ARGS "-i ${RC_FILE} -o ${RES_FILE} --include-dir=${CMAKE_CURRENT_SOURCE_DIR}/images")
        list (APPEND SOURCES ${RES_FILE} )
    else ()
        list (APPEND SOURCES ${RC_FILE} )
    endif ()
endif ()

if (APPLE)
    set ( ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/images/qsynth.icns )
    list ( APPEND SOURCES ${ICON_FILE} )
    set ( MACOSX_BUNDLE_ICON_FILE qsynth.icns )
    set_source_files_properties ( ${ICON_FILE} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources )
endif ()

add_executable ( qsynth
    ${UI_SOURCES}
    ${MOC_SOURCES}
    ${QRC_SOURCES}
    ${SOURCES}
)

if (WIN32)
    set_target_properties ( qsynth PROPERTIES WIN32_EXECUTABLE true )
endif ()

if (APPLE)
    set_target_properties ( qsynth PROPERTIES MACOSX_BUNDLE true )
endif ()

target_link_libraries ( qsynth
    ${QT_LIBRARIES}
    ${MATH_LIBRARY}
    ${FLUIDSYNTH_LIBRARY}
    ${X11_LIBRARY}
)
qt5_use_modules (qsynth Core Gui Widgets X11Extras)

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

if (UNIX AND NOT APPLE)
    install ( TARGETS qsynth
              RUNTIME DESTINATION bin )
    install ( FILES ${QM_FILES}
              DESTINATION share/qsynth/translations )
    install ( FILES ${CMAKE_CURRENT_BINARY_DIR}/qsynth.desktop
              DESTINATION share/applications )
    install ( FILES images/qsynth.png
              DESTINATION share/pixmaps )
endif ()

if (WIN32)
    install ( TARGETS qsynth
              RUNTIME DESTINATION . )
    install ( FILES ${QM_FILES}
              DESTINATION share/qsynth/translations )
endif ()

