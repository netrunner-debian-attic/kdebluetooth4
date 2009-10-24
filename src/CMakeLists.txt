if (NEPOMUK_FOUND)
	include_directories(${NEPOMUK_INCLUDES})
endif(NEPOMUK_FOUND)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${KDE4_INCLUDES}
)

add_subdirectory(inputwizard)
add_subdirectory(device-manager)
 
set(kbluetray_SRCS 
	main.cpp
	trayicon.cpp
	adapterconfig.cpp	
	adapterwidget.cpp
	obexsession/obexsession.cpp
	obexsession/obexsessionfiletransfer.cpp
	deviceselector/devicesel.cpp
	obexserver/obexserver.cpp
	obexserver/obexserversession.cpp
	obexserver/obexserversessionfiletransfer.cpp
    agent/agent.cpp
	agent/passkeydialog.cpp
	agent/authdialog.cpp
	agent/confirmdialog.cpp
	kbluelock.cpp
)

kde4_add_ui_files(kbluetray_SRCS
			receiveDialogWidget.ui
            adapterConf.ui
			adapterWidget.ui
			agent/authdialog.ui
			agent/passkeydialog.ui
			agent/confirmdialog.ui
			device-manager/deviceman.ui
			deviceselector/deviceselector.ui)

kde4_add_executable(kbluetooth ${kbluetray_SRCS})

configure_file(config-nepomuk.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-nepomuk.h)

# If major than 4.3.9 which means 4.4.0 link against KDE4_KDEUI
if(${KDE_VERSION} VERSION_GREATER 4.3.9)
	target_link_libraries(kbluetooth
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_KFILE_LIBS} solidcontrol)
elseif(${KDE_VERSION} VERSION_GREATER 4.2.9 AND ${KDE_VERSION} VERSION_LESS 4.4.0)
	target_link_libraries(kbluetooth
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_KFILE_LIBS} solidcontrol knotificationitem-1)
endif(${KDE_VERSION} VERSION_GREATER 4.3.9)

if(HAVE_NEPOMUK)
	target_link_libraries(kbluetooth ${NEPOMUK_LIBRARIES})
endif(HAVE_NEPOMUK)

install(TARGETS kbluetooth
        DESTINATION ${KDE4_BIN_INSTALL_DIR})

#install( FILES pics/search.gif DESTINATION ${DATA_INSTALL_DIR}/kbluetooth4 )

install(FILES kbluetooth.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

