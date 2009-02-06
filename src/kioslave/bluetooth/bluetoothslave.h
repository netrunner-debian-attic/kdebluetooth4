/*  This file is part of the KDE project
    Copyright (C) 2007 Juan González Aguilera <kde_devel@opsiland.info>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
*/

#ifndef __BLUETOOTH_SLAVE__
#define __BLUETOOTH_SLAVE__

#include <kio/slavebase.h>
#include <kurl.h>
#include <QObject>
#include <QEventLoop>
#include <QString>
#include <solid/control/bluetoothinterface.h>

/**
 * Is the responsible of providing a central point to the bluetooth protocol handling for the KIO subsystem.
 * We have three posibble URLs:
 *  - bluetooth://                     : lists the remote devices that are on range of the default system bluetooth device. 
 *  - bluetooth:/[Device]/             : lists the services available on the given device. Device must be a UBI.
 *  - bluetooth:/[Device]/service-name : must open a handler for the given service, how to do this is still undefined
 *
 * TODO: Some systems might have more than one bluetooth adapter, how do we handle that?
 * TODO: Depending on the previous, Device will include the path to the local device or not
 * TODO: Current implementation requires the URL to include the Bluetooth Device Address, and doesn't support aliases.
 * TODO: Define how to handle a given service
 *  
 */
class BluetoothSlave: public QObject, public KIO::SlaveBase
{
	Q_OBJECT
	public:
		/**
		 * Calls KIO::SlaveBase constructor to set up KIO stuff and finds the default local device.
		 * @param pool_socket For SlaveBase
		 * @param app_socket For SlaveBase
		 */
		BluetoothSlave(const QByteArray &pool_socket, const QByteArray &app_socket);
		virtual ~BluetoothSlave();
		
		virtual void stat(const KUrl &url);
		virtual void listDir(const KUrl &url);
	private slots:		
		void slotAddDevice(const QString &ubi, int devclass, int rssi);
		void slotEndDiscover();
				
		void slotAddService(const QString &ubi, const Solid::Control::BluetoothServiceRecord &record);
		void slotServiceDiscoveryFinished();
		
		/**
		 * This is scheduled to execution using QTimer::singleShot, and just exit() the QEventLoop
		 */
		void slotFinishEventLoop();
		
	private:
		/**
		 * Constructs the mimetype based on the majorClass. Then creates the entry and lists it using listEntry.
		 * If last is set to true, repeats the entry, with the second argument of listEntry set to true
		 * @param url The url of the remote device
		 * @param name Name to be shown to the user
		 * @param majorClass Indicates which kind of device is been listed, computer, telephone,...
		 * @param last If set to true, repeats the last entry to indicate listing is done.
		 */
		void listRemoteDevice(const QString &url, const QString &name, const QString &majorClass,bool last=false);
		/**
		 * Constructs a UDSEntry from the given parameters. If mimetype indicates that the entry is a directory,
		 * then associates KIO::UDS_FILE_TYPE with S_IFDIR. Also set full read access for all users.
		 * @param title Inserted with the KIO::UDS_NAME key
		 * @param url Inserted with the KIO::UDS_URL key
		 * @param mimeType Inserted with the KIO::UDS_MIME_TYPE key
		 * @return The UDSEntry that has been filled accordig to the parameters
		 */
		KIO::UDSEntry createDirEntry(const QString &title, const QString &url, const QString &mimeType = QString::fromLatin1("inode/directory"));
		/**
		 * Checks if the given url contains a valid, and available, hciN device identifier,
		 * If it's valid it updates the information about the current local interface.
		 * This method will try to use the default adapter if it can't find the requested
		 * adapter. If there is no available local interface it will emit an error indicating it.
		 * It will also set currentLocal if it's found on the url, if not found empties it.
		 * @param url Of the form bluetooth:/hciN/UBI/...
		 */
		void updateCurrent(const KUrl &url);
		/**
		 * Lists the remote devices that Solid is aware of for currentRemote.
		 * Uses the service name to assign the mime type, the url and the node name.
		 * @param url Used to create uniform urls that include the service-name
		 */
		void listServices(const KUrl &url);
		
		/// The current local interface. Actually this is the default adapter
		Solid::Control::BluetoothInterface *interface;
		
		/// The current remote
		Solid::Control::BluetoothRemoteDevice remote;
		
		/// Needed to receive signals from the Solid layer
		QEventLoop loop;
		
		/// Complete ubi of the current remote device from the URL, if there is any
		QString currentRemote;
		
		/// Service name with spaces substituted by dashes
		QString currentService;
		
		KIO::UDSEntryList serviceList;
};


#endif
