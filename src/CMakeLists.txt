include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

add_subdirectory(inputwizard)
add_subdirectory(device-manager)

#if the Agent should run as stand alone app, uncomment this line
#add_subdirectory(agent)

 
set(kbluetray_SRCS 
	main.cpp
	trayicon.cpp
	adapterconfig.cpp	
	adapterwidget.cpp
#	inputwizard/wizarddialog.cpp
#	inputwizard/wizardpage.cpp
#	device-manager/devicemanager.cpp
	obexsession/obexsession.cpp
	deviceselector/devicesel.cpp
	obexserver/obexserver.cpp
	obexserver/obexserversession.cpp
        agent/agent.cpp
 	agent/passkeydialog.cpp
	agent/authdialog.cpp
)


kde4_add_ui_files(kbluetray_SRCS
                        adapterConf.ui
			adapterWidget.ui
			agent/authdialog.ui
			agent/passkeydialog.ui
			device-manager/deviceman.ui
			deviceselector/deviceselector.ui)

kde4_add_executable(kbluetooth4 ${kbluetray_SRCS})

target_link_libraries(kbluetooth4
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_KFILE_LIBS} solidcontrol)
 
install(TARGETS kbluetooth4
        DESTINATION ${KDE4_BIN_INSTALL_DIR})

#install( FILES pics/search.gif DESTINATION ${DATA_INSTALL_DIR}/kbluetooth4 )

install(FILES kdebluetooth4.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

