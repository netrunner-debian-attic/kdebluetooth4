include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )


set(deviceman_SRCS 
	main.cpp
	devicemanager.cpp
	devicemanager.h
)


kde4_add_ui_files(deviceman_SRCS
                        deviceman.ui
)

kde4_add_executable(kbluetooth-devicemanager ${deviceman_SRCS})

target_link_libraries(kbluetooth-devicemanager
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4WORKSPACE_SOLIDCONTROL_LIBRARY})
 
install(TARGETS kbluetooth-devicemanager
        DESTINATION ${KDE4_BIN_INSTALL_DIR})

