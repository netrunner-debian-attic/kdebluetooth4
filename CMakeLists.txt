project(kbluetooth4)
 
# set version
set (KBLUETOOTH_VERSION_MAJOR 1)
set (KBLUETOOTH_VERSION_MINOR 0)
set (KBLUETOOTH_VERSION_RELEASE 00)
set (KBLUETOOTH_VERSION "${KBLUETOOTH_VERSION_MAJOR}.${KBLUETOOTH_VERSION_MINOR}.${KBLUETOOTH_VERSION_RELEASE}" )
set (KBLUETOOTH_VERSION_STRING "${KBLUETOOTH_VERSION} (KBluetooth 1.0)")

#set (CMAKE_INSTALL_PREFIX /usr)
SET( QT_USE_QT3SUPPORT TRUE )

# Find the required Libaries
find_package(KDE4 REQUIRED)
find_package(Qt4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   ${QT_QT3SUPPORT_INCLUDE_DIR}
    
   )
add_subdirectory ( src )
add_subdirectory( icons )
