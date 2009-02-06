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

#include "bluetoothslave.h"
#include <kcomponentdata.h>
#include <klocale.h>
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothremotedevice.h>
#include <qcoreapplication.h>
#include <QTimer>

// #include "sdp.h"

using namespace Solid::Control;
using namespace KIO;

QRegExp regexAddress("([a-fA-F0-9][a-fA-F0-9]:){5}[a-fA-F0-9][a-fA-F0-9]");

extern "C"
{
	KDE_EXPORT
	int kdemain(int argc, char **argv)
	{
		KComponentData cData( "kio_bluetooth" );
		QCoreApplication qapp(argc,argv);
		kdDebug() << "*** Starting kio_bluetooth " << endl;
		if (argc != 4)
		{
			kdDebug() << "Usage: kio_bluetooth  protocol domain-socket1 domain-socket2" << endl;
			exit(-1);
		}
		BluetoothSlave slave(argv[2], argv[3]);
		slave.dispatchLoop();		
		kdDebug(7101) << "*** kio_bluetooth Done" << endl;
		return 0;
	}
}

BluetoothSlave::BluetoothSlave(const QByteArray & pool_socket, const QByteArray & app_socket)
	: SlaveBase("kio_bluetooth",pool_socket,app_socket)
{
	interface = 0;	
	BluetoothManager &bzManager = BluetoothManager::self();	
	QString ubi = bzManager.defaultInterface();
	if(ubi.isEmpty())
		error(KIO::ERR_DOES_NOT_EXIST,i18n("No bluetooth adapter was detected"));
	else
		interface = new BluetoothInterface(ubi);
}

BluetoothSlave::~ BluetoothSlave()
{
	delete interface;
	interface = 0;	
}

void BluetoothSlave::stat(const KUrl & url)
{
	kdDebug() << "Stat of " << url << endl;
	updateCurrent(url);
	
	if(!currentRemote.isEmpty()) //Want to stat some remote device
	{
		if(currentService.isEmpty())
			statEntry(createDirEntry(currentRemote,url.url()));//FIXME Add mimetype, do we need some map for this?
		else
			statEntry(createDirEntry(currentService.replace("_"," "),url.url(),"bluetooth/"+currentService));
	} else
	{
		statEntry ( createDirEntry(i18n( "Bluetooth Neighborhood" ),"bluetooth:/"));//FIXME i18n
	}
	finished();
}

void BluetoothSlave::listDir(const KUrl & url)
{
	kdDebug() << "Listing dir " <<url<< endl;
	updateCurrent(url);
	
	if(!currentRemote.isEmpty()) //Want to find remote services
	{
		kdDebug() << "Listing services for " << currentRemote << endl;
		remote = interface->findBluetoothRemoteDevice(currentRemote);
		QObject::connect(&remote,SIGNAL(remoteServiceFound(const QString &,const Solid::Control::BluetoothServiceRecord &)),
				  this,SLOT(slotAddService(const QString &, const Solid::Control::BluetoothServiceRecord &)));
		QObject::connect(&remote,SIGNAL(serviceDiscoveryFinished()),
				  this,SLOT(slotServiceDiscoveryFinished()));
		serviceList.clear();
		remote.findServices();
	} else //Want to list remote devices
	{
		kdDebug() << "Listing remote devices"<< endl;
		QObject::connect(interface,SIGNAL(remoteDeviceFound( const QString&, int, int )),
				 this,SLOT(slotAddDevice(const QString&, int, int)));
		QObject::connect(interface,SIGNAL(discoveryCompleted()),
				 this,SLOT(slotEndDiscover()));
				
		interface->discoverDevices();
	}
	kDebug() << "Waiting for listDir to end" << endl;
	loop.exec();	// This will block until slotEndDevice is called
	finished();
	kdDebug() << "Finished listing dir" << endl;
}

void BluetoothSlave::listServices(const KUrl &)
{
	
}

void BluetoothSlave::slotAddDevice(const QString & ubi, int /*devclass*/, int /*rssi*/)
{
	kdDebug() << "New remote device found, UBI: " << ubi << endl;
	BluetoothRemoteDevice remote= interface->findBluetoothRemoteDevice(ubi);
	QString name = remote.name();
	QString alias = remote.alias();
	QString mclass = remote.majorClass();	
	if ( !alias.isEmpty() )
	{
		name = alias;
	}
	else if ( name.isEmpty() )
	{
		name = ubi;		
	}
	regexAddress.indexIn(ubi);
	QString url = "bluetooth:/[" + regexAddress.cap() + "]/";
	
	listRemoteDevice(url,name,mclass);//last=false
}
void BluetoothSlave::listRemoteDevice(const QString &url, const QString & name, const QString & majorClass,bool last)
{
	QString mimeType = "bluetooth/"+ majorClass+"-device-class";
	
	kdDebug() << "Listing: " << url << " with mimeType " << mimeType << endl;
	
	UDSEntry entry = createDirEntry (name,url,mimeType);
	
	listEntry(entry,false);
	if(last)
		listEntry(entry,true);
}
KIO::UDSEntry BluetoothSlave::createDirEntry(const QString & title, const QString & url, const QString & mimeType)
{
	UDSEntry entry;
	entry.clear();	//FIXME This may be redundant
	
// 	kdDebug() << "Creating entry: " << title << " " << url << " " << mimeType << endl;	
	Q_ASSERT(!title.isEmpty());
	entry.insert(KIO::UDS_NAME,title);
 	
	Q_ASSERT(!url.isEmpty());
	entry.insert(KIO::UDS_URL,url);
	
	Q_ASSERT(!mimeType.isEmpty());
	entry.insert(KIO::UDS_MIME_TYPE,mimeType);
	
	if(mimeType=="inode/directory") //FIXME this must be single-stored somewhere
		entry.insert(KIO::UDS_FILE_TYPE,S_IFDIR);
	entry.insert(KIO::UDS_ACCESS,S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	return entry;
}

void BluetoothSlave::slotEndDiscover()
{
	listRemoteDevice("bluetooth:/localhost/","localhost","computer",true);	
	
	QObject::disconnect(interface,SIGNAL(remoteDeviceFound( const QString&, int, int )),
			    this,SLOT(slotAddDevice(const QString&, int, int)));
	QObject::disconnect(interface,SIGNAL(discoveryCompleted()),this,SLOT(slotEndDiscover()));
	
	QTimer::singleShot(100,this,SLOT(slotFinishEventLoop()));
	
	kdDebug() << "Remote device discovery finished" << endl;
}

void BluetoothSlave::slotFinishEventLoop()
{
	loop.exit();
// 	this->exit();
}

void BluetoothSlave::updateCurrent(const KUrl & url)
{
	kdDebug() << "Url received is " << url << endl;
	QString path = url.path(KUrl::RemoveTrailingSlash).remove(0,1);
	currentRemote.clear();
	int index;
	if(!path.isEmpty() && (index=regexAddress.indexIn(path))!=-1)
	{
		currentRemote = interface->ubi() + "/" + regexAddress.cap();
		kdDebug() << "Updating current remote : " << currentRemote << endl;
		
		QStringList list = path.split("/");
		if(list.size()>1)
			currentService = list.last();
		else
			currentService.clear();
		kdDebug() << "Updating current service : " << currentService << endl;
	}
		
	
}

void BluetoothSlave::slotAddService(const QString & ubi, const Solid::Control::BluetoothServiceRecord & record)
{
	regexAddress.indexIn(ubi);
	serviceList << createDirEntry(record.name,"bluetooth:/["+regexAddress.cap() +"]/"+QString(record.name).replace(" ","_"),"text/text");//FIXME Which mime type to use?	
}

void BluetoothSlave::slotServiceDiscoveryFinished()
{
	kDebug() << "Finished searching services" << endl;
	listEntries(serviceList);
	QObject::disconnect(&remote,SIGNAL(remoteServiceFound(const QString &,const Solid::Control::BluetoothServiceRecord &)),
			  this,SLOT(slotAddService(const QString &, const Solid::Control::BluetoothServiceRecord &)));
	QObject::disconnect(&remote,SIGNAL(serviceDiscoveryFinished()),
			  this,SLOT(slotServiceDiscoveryFinished()));	
	QTimer::singleShot(100,this,SLOT(slotFinishEventLoop()));
}


#include "bluetoothslave.moc"
