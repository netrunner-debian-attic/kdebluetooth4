include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )


 
set(agent_SRCS 
	main.cpp
	agent.cpp
        passkeydialog.cpp
        authdialog.cpp
)


kde4_add_ui_files(agent_SRCS
			authdialog.ui
			passkeydialog.ui)


kde4_add_executable(agent ${agent_SRCS})

target_link_libraries(agent
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_KFILE_LIBS} solidcontrol)
 
install(TARGETS agent
        DESTINATION ${KDE4_BIN_INSTALL_DIR})

