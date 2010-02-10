project(kbluetooth)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
 
# set version
set (KBLUETOOTH_VERSION_MAJOR 1)
set (KBLUETOOTH_VERSION_MINOR 0)
set (KBLUETOOTH_VERSION_RELEASE 00)
set (KBLUETOOTH_VERSION "${KBLUETOOTH_VERSION_MAJOR}.${KBLUETOOTH_VERSION_MINOR}.${KBLUETOOTH_VERSION_RELEASE}" )
set (KBLUETOOTH_VERSION_STRING "${KBLUETOOTH_VERSION} (KBluetooth 1.0)")

#set (CMAKE_INSTALL_PREFIX /usr)
# SET( QT_USE_QT3SUPPORT TRUE )

# Find the required Libaries
find_package(Qt4 REQUIRED)
find_package(KDE4 4.3.0 REQUIRED)
find_package(KDE4Workspace REQUIRED)

#In kde 4.3 libKNotification was experimental
if(${KDE_VERSION} VERSION_LESS 4.3.9)
	find_package(LibKNotificationItem-1 REQUIRED)
	set (KBLUETOOTH_LIBKNOTIFICATION ${KNOTIFICATIONITEM_LIBRARIES})
endif(${KDE_VERSION} VERSION_LESS 4.3.9)

macro_optional_find_package(Nepomuk)
macro_log_feature(NEPOMUK_FOUND "Nepomuk" "Handles all kinds of metadata on the KDE desktop to provide a semantic desktop" "http://nepomuk.kde.org/" FALSE "" "STRONGLY_RECOMMENDED: Nepomuk is used to provide metadata on objects like files, emails etc. to provide a semantic desktop.")
macro_bool_to_01(NEPOMUK_FOUND HAVE_NEPOMUK)
macro_display_feature_log()

include(KDE4Defaults)
include(MacroLibrary)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
add_subdirectory ( src )
add_subdirectory( icons )
find_package(Msgfmt REQUIRED)
find_package(Gettext REQUIRED)
add_subdirectory( po )
add_subdirectory( doc-translations )
