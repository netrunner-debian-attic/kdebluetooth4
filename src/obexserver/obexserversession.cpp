/*
 *
 *  KBluetooth4 - KDE Bluetooth Framework
 *
 *  Copyright (C) 2008  Tom Patzig <tpatzig@suse.de>
 *
 *  This file is part of kbluetooth4.
 *
 *  kbluetooth4 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  kbluetooth4 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with kbluetooth4; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
*/

#include "obexserversession.h"

#include <QVariant>
#include <KDebug>

ObexServerSession::ObexServerSession(QObject* parent, const QString& path) : m_parent(parent)
{

	QDBusConnection* dbus = new QDBusConnection("dbus");
	QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");

	obexService = "org.openobex";
	QString sessionPath = path;
	QString sessionIface = "org.openobex.ServerSession";

	manager = new QDBusInterface(obexService,sessionPath,sessionIface,dbusconn);

	kDebug() << "obex server session: " << sessionPath;

	dbusconn.connect("",sessionPath,sessionIface,"Cancelled",this,SLOT(slotCancelled()));		
	dbusconn.connect("",sessionPath,sessionIface,"Disconnected",this,SLOT(slotDisconnected()));
	dbusconn.connect("",sessionPath,sessionIface,"TransferStarted",this,SLOT(slotTransferStarted(const QString&, const QString&, qulonglong)));
	dbusconn.connect("",sessionPath,sessionIface,"TransferProgress",this,SLOT(slotTransferProgress(qulonglong)));
	dbusconn.connect("",sessionPath,sessionIface,"TransferCompleted",this,SLOT(slotTransferCompleted()));
	dbusconn.connect("",sessionPath,sessionIface,"ErrorOccurred",this,SLOT(slotErrorOccurred(const QString&, const QString&)));
  

}

ObexServerSession::~ObexServerSession() 
{
	delete manager;
}

void ObexServerSession::accept() {
	manager->call("Accept");
}

void ObexServerSession::reject() {
	manager->call("Reject");
}

void ObexServerSession::disconnect() {
	kDebug() << "obex server session disconnected";
	manager->call(QDBus::Block, "Disconnect");
}

QMap<QString,QVariant> ObexServerSession::getTransferInfo() {
	QDBusReply< QMap<QString,QVariant> > reply = manager->call("GetTransferInfo");
	if (reply.isValid())
		return reply.value();
	else
		return QMap<QString, QVariant>();
}

void ObexServerSession::cancel() {
	kDebug() << "obex server session canceled";
	manager->call(QDBus::Block, "Cancel");
}

//SLOTS to emit the signals

void ObexServerSession::slotCancelled()
{
	emit cancelled();
}

void ObexServerSession::slotDisconnected()
{
	emit disconnected();
}

void ObexServerSession::slotTransferStarted(const QString& filename, const QString& local_path, qulonglong total_bytes)
{
	emit transferStarted(filename,local_path,total_bytes);
}

void ObexServerSession::slotTransferProgress(qulonglong bytes_transferred)
{
	emit transferProgress(bytes_transferred);
}

void ObexServerSession::slotTransferCompleted()
{
	emit transferCompleted();
}

void ObexServerSession::slotErrorOccurred(const QString& error_name, const QString& error_message)
{
	emit errorOccurred(error_name,error_message);
}
