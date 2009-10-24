include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)
 
# We add our source code here
set(inputwizard_SRCS 
	main.cpp
	wizarddialog.cpp
	wizardpage.cpp
	../agent/agent.cpp
	../agent/passkeydialog.cpp
	../agent/authdialog.cpp
	../agent/confirmdialog.cpp
)

kde4_add_ui_files(inputwizard_SRCS
			../agent/authdialog.ui
			../agent/passkeydialog.ui
			../agent/confirmdialog.ui
)

kde4_add_executable(kbluetooth-inputwizard ${inputwizard_SRCS})

target_link_libraries(kbluetooth-inputwizard
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)
 
install(TARGETS kbluetooth-inputwizard
        DESTINATION ${KDE4_BIN_INSTALL_DIR})

