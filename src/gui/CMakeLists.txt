
FILE(GLOB_RECURSE hydrogen_SRCS src/*.cpp src/*.h)

# add undiscovered dependencies to generated config.h
INCLUDE(AddFileDependencies)
FOREACH( _file ${hydrogen_H} ${hydrogen_SRCS})
    FILE(READ "${_file}" _FILE_CONTENT)
    IF( "${_FILE_CONTENT}" MATCHES "H2CORE_HAVE_" )
        ADD_FILE_DEPENDENCIES( ${_file} ${CMAKE_BINARY_DIR}/src/core/config.h)
    ENDIF()
ENDFOREACH()

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}                 # ui_ headers
    ${CMAKE_CURRENT_SOURCE_DIR}/src             # gui headers
    ${CMAKE_BINARY_DIR}/src                     # generated config.h
    ${CMAKE_SOURCE_DIR}/src                     # top level headers
    ${QT_INCLUDES}
    ${LASH_INCLUDE_DIRS}
    ${LIBARCHIVE_INCLUDE_DIRS}
    ${LIBSNDFILE_INCLUDE_DIRS}
    ${JACK_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIRS}
)

IF(APPLE)
	# set how it shows up in the Info.plist file
     	SET(MACOSX_BUNDLE_INFO_STRING "Hydrogen for Mac")
	SET(MACOSX_BUNDLE_ICON_FILE "icon.icns")
	SET(MACOSX_BUNDLE_GUI_IDENTIFIER "Hydrogen")
	SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION}")
	#this has to be short since it appears as the application name in the menu bar
	SET(MACOSX_BUNDLE_BUNDLE_NAME "Hydrogen")
	SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}")
	SET(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION}")
	SET(MACOSX_BUNDLE_COPYRIGHT "Hydrogen Developers, 2009")

  	# set where in the bundle to put the icns file
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/data/img/gray/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
	# include the icns file in the target
	SET(hydrogen_SRCS ${hydrogen_SRCS} ${CMAKE_SOURCE_DIR}/data/img/gray/icon.icns)
	
	#copy the data folder
    FILE(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION hydrogen.app/Contents/Resources PATTERN ".svn" EXCLUDE PATTERN i18n EXCLUDE)
  	
ENDIF()

ADD_EXECUTABLE(hydrogen WIN32 MACOSX_BUNDLE ${hydrogen_SRCS} ${hydrogen_MOC} ${hydrogen_UIS_H} ${CMAKE_SOURCE_DIR}/windows/icon.rc)
SET_PROPERTY(TARGET hydrogen PROPERTY CXX_STANDARD 17)
if(APPLE)
    add_custom_command(
        TARGET hydrogen
        POST_BUILD
        COMMAND plutil -replace NSPrincipalClass -string NSApplication hydrogen.app/Contents/Info.plist
    )
    add_custom_command(
        TARGET hydrogen
        POST_BUILD
        COMMAND plutil -replace NSHighResolutionCapable -bool true hydrogen.app/Contents/Info.plist
    )
    add_custom_command(
        TARGET hydrogen
        POST_BUILD
        COMMAND
            plutil -extract CFBundleDocumentTypes json
                   -o /dev/null
                   hydrogen.app/Contents/Info.plist >/dev/null
            || plutil -insert CFBundleDocumentTypes
                      -json
                         \"` cat ${CMAKE_SOURCE_DIR}/macos/CFBundleDocumentTypes.json `\"
                       hydrogen.app/Contents/Info.plist )

    ADD_FILE_DEPENDENCIES(hydrogen ${CMAKE_SOURCE_DIR}/macos/CFBundleDocumentTypes.json)

endif()

TARGET_LINK_LIBRARIES(hydrogen
	hydrogen-core-${VERSION}
	Qt5::Widgets
	Qt5::Svg
)

IF(Backtrace_FOUND)
	TARGET_LINK_LIBRARIES(hydrogen
		${Backtrace_LIBRARIES}
	)
ENDIF()

# Precompiled headers for Qt and std::vector. These dramatically reduce GUI build times
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.16)
	target_precompile_headers(hydrogen
		PUBLIC
			<QtGui>
			<QtWidgets>
			<vector> )
endif()


ADD_DEPENDENCIES(hydrogen hydrogen-core-${VERSION})

INSTALL(TARGETS hydrogen RUNTIME DESTINATION ${H2_BIN_PATH} BUNDLE DESTINATION ${H2_BIN_PATH})
