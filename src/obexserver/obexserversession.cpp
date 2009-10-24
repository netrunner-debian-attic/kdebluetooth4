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

#include "obexserversession.h"
#include "obexserversessionfiletransfer.h"
#include "obexserver.h"

#include <QVariant>

#include <KDebug>
#include <KJob>

ObexServerSession::ObexServerSession(QObject* parent, ObexServer* obexServer, const QString& path) : m_parent(parent)
{
	m_obexServer = obexServer;

	m_path = path;
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
	dbusconn.connect("",sessionPath,sessionIface,"ErrorOccurred",this,SLOT(slotErrorOccurred(const QString&, const QString&)));

	delete dbus;
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
	fileTransfer->feedbackReceived();
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

QString ObexServerSession::path() {
	return m_path;
}

ObexServer* ObexServerSession::server() {
	return m_obexServer;
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
	fileTransfer = new ObexServerSessionFileTransfer(this, manager, filename, local_path, total_bytes);
	emit transferStarted(fileTransfer);
}

void ObexServerSession::slotErrorOccurred(const QString& error_name, const QString& error_message)
{
	emit errorOccurred(error_name,error_message);
}