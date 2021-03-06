/***************************************************************************
 *   Copyright (C) 2008  Tom Patzig <tpatzig@suse.de>                      *
 *   Copyright (C) 2008  Alex Fiestas <alex@eyeos.org>                     *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "obexsession.h"
#include "obexsessionfiletransfer.h"

#include <QVariant>

#include <KDebug>
// #include <KMessageBox>

ObexSession::ObexSession(QObject* parent, const QString& srcAddr, const QString& addr, const QString& service) : error(false), m_parent(parent)
{
	session = manager = 0;
	m_targetAddress = addr;

	QDBusConnection* dbus = new QDBusConnection("dbus");
	QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");
	
	if(!dbusconn.isConnected()){
		error = true;
		return;
	}
	obexService = "org.openobex";
	QString path = "/org/openobex";
	QString method = "CreateBluetoothSession";
	QString iface = "org.openobex.Manager";

	kDebug() << "Konstruktor: " << path;

	manager = new QDBusInterface(obexService,path,iface,dbusconn);
	
	if(!manager->isValid()){
		error = true;
		return;
	}
	
// 	**synchronous call**
//	QDBusReply<QDBusObjectPath> reply = manager->call(method,addr,service);
	QList<QVariant> args;
	args << addr << srcAddr << service;

	manager->callWithCallback( method, args, this, SLOT(sessionCreated(QDBusObjectPath)), SLOT(sessionCreatedError(QDBusError)) );
	dbusconn.connect("",path,iface,"SessionConnected",this,SLOT(slotConnected(QDBusObjectPath)));	
	dbusconn.connect("",path,iface,"SessionConnectError",this,SLOT(slotConnectError(QDBusObjectPath,QString,QString)));	
}

ObexSession::~ObexSession() 
{
	if(session)
		delete session;
	
	if(manager)
		delete manager;
}

void ObexSession::sessionCreated(QDBusObjectPath path)
{
	QDBusConnection* dbus = new QDBusConnection("dbus");
	QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");

	sessionPath = path.path();

	sessionIface = "org.openobex.Session";

	session = new QDBusInterface(obexService, sessionPath, sessionIface, dbusconn);
	if(!session->isValid()){
		error = true;
		emit openObexError();
		return;
	}
	kDebug() << "session interface created for: " << sessionPath ;

// 	connect the DBus Signal to slots

	dbusconn.connect("",sessionPath,sessionIface,"Cancelled",this,SLOT(slotCancelled()));	
	dbusconn.connect("",sessionPath,sessionIface,"Disconnected",this,SLOT(slotDisconnected()));	
	dbusconn.connect("",sessionPath,sessionIface,"Closed",this,SLOT(slotClosed()));	
	//dbusconn->connect("",sessionPath,sessionIface,"TransferStarted",this,SLOT(slotTransferStarted(const QString&, const QString&, qulonglong)));
	//dbusconn->connect("",sessionPath,sessionIface,"TransferProgress",this,SLOT(slotTransferProgress(qulonglong)));
	//dbusconn->connect("",sessionPath,sessionIface,"TransferCompleted",this,SLOT(slotTransferCompleted()));
	dbusconn.connect("",sessionPath,sessionIface,"ErrorOccurred",this,SLOT(slotErrorOccurred(const QString&, const QString&)));
}

void ObexSession::sessionCreatedError(QDBusError err)
{
	kDebug() << "Error creating Bluetooth Session: " << err.message();
// 	KMessageBox::error(0 , err.message(), "Error creating Bluetooth Session");
}

void ObexSession::connect()
{
	session->call("Connect");
}

void ObexSession::disconnect()
{
	kDebug() << "CAlling disconnect";
	session->call("Disconnect");
	session->call("Close");
}

void ObexSession::close()
{
	session->call("Close");
}

bool ObexSession::isConnected()
{
	QDBusReply<bool> reply = session->call("IsConnected");
	if (reply.isValid())
		return reply.value();
	else
		return false;
}

void ObexSession::changeCurrentFolder(const QString& path)
{
	session->call("ChangeCurrentFolder",path);
}

void ObexSession::changeCurrentFolderBackward()
{
	session->call("ChangeCurrentFolderBackward");
}

void ObexSession::changeCurrentFolderToRoot()
{
	session->call("ChangeCurrentFolderToRoot");
}

QString ObexSession::getCurrentPath()
{
	QString val;
	QDBusReply<QString> reply = session->call("GetCurrentPath");
	if (reply.isValid()) {
		val = reply.value();
		return val;
	} else
		return "";

}

void ObexSession::copyRemoteFile(const QString& remote_filename, const QString& local_path)
{
	session->call("CopyRemoteFile",remote_filename,local_path);

}

void ObexSession::createFolder(const QString& folder_name)
{
	session->call("CreateFolder",folder_name);

}

QMap<QString,QVariant> ObexSession::retrieveFolderListing()
{
	QDBusReply< QMap<QString,QVariant> > reply = session->call("RetrieveFolderListing");
	if (reply.isValid())
		return reply.value();
	else
		return QMap<QString,QVariant>();

}

QMap<QString,QVariant> ObexSession::getCapability()
{
	QDBusReply< QMap<QString,QVariant> > reply = session->call("GetCapability");
	if (reply.isValid())
		return reply.value();
	else
		return QMap<QString,QVariant>();

}

void ObexSession::sendFile(const QString& local_path, bool yes)
{
	Q_UNUSED(yes);
	session->call("SendFile",local_path);
}

KJob* ObexSession::sendFile(const QString& localPath) {
	KJob* fileTransfer = new ObexSessionFileTransfer(this, session, localPath);
	return fileTransfer;
}

void ObexSession::deleteRemoteFile(const QString& remote_filename)
{
	session->call("DeleteRemoteFile",remote_filename);
}

QMap<QString,QVariant> ObexSession::getTransferInfo()
{
	QDBusReply< QMap<QString,QVariant> > reply = session->call("GetTransferInfo");
	if (reply.isValid())
		return reply.value();
	else
		return QMap<QString, QVariant>() ;
}

bool ObexSession::isBusy()
{
	QDBusReply<bool> reply = session->call("IsBusy");
	if (reply.isValid())
		return reply.value();
	else
		return false;
}

void ObexSession::cancel()
{
	session->call("Cancel");
}

QString ObexSession::targetAddress() {
	return m_targetAddress;
}

//SLOTS to emit the signals
void ObexSession::slotCancelled()
{
	emit cancelled();
}

void ObexSession::slotConnected(QDBusObjectPath path)
{
	kDebug() << "Session connected " <<  path.path();
	emit connected();
}

void ObexSession::slotConnectError(QDBusObjectPath path, QString err_name, QString err_msg)
{
	Q_UNUSED(path);
	kDebug() << "Session Connect Error " << err_name << " " << err_msg;
}

void ObexSession::slotDisconnected()
{
	emit disconnected();
}

void ObexSession::slotClosed()
{
	emit closed();
}

void ObexSession::slotTransferStarted(const QString& filename, const QString& local_path, qulonglong total_bytes)
{
	emit transferStarted(filename,local_path,total_bytes);
}

void ObexSession::slotTransferProgress(qulonglong bytes_transferred)
{
	emit transferProgress(bytes_transferred);
}

void ObexSession::slotTransferCompleted()
{
	emit transferCompleted();
}

void ObexSession::slotErrorOccurred(const QString& error_name, const QString& error_message)
{
	emit errorOccurred(error_name,error_message);
}