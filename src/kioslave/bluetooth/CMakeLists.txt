PROJECT(bluetoothslave)

set(bluetoothslave_SOURCES bluetoothslave.cpp)

KDE4_AUTOMOC(${bluetoothslave_SOURCES})
KDE4_ADD_PLUGIN(kio_bluetooth ${bluetoothslave_SOURCES})
TARGET_LINK_LIBRARIES(kio_bluetooth ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} solidcontrol)

#FIXME: Hardcoded install path
#I should use ${SERVICES_INSTALL_DIR} but it's not working
#i.e. reports a totally different directory than the real one
INSTALL(FILES bluetoothslave.protocol DESTINATION /opt/kde4/share/kde4/services/)
#Same but with ${PLUGIN_INSTALL_DIR}
INSTALL(TARGETS kio_bluetooth DESTINATION /opt/kde4/lib/kde4/)
