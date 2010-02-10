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

#include "obexserver.h"

#include <QVariant>
#include <QDBusObjectPath>

#include <KDebug>
// #include <KMessageBox>
#include <KLocale>

typedef QMap<QString, QString> StringMapReply;
Q_DECLARE_METATYPE(StringMapReply)

ObexServer::ObexServer(QObject* parent, const QString& addr, const QString& pattern, bool require_pairing) : error(false), m_parent(parent)
{
	session = 0;

	QDBusConnection* dbus = new QDBusConnection("dbus");
	QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");

	//Dbus must be pressent and this connection can't fail
	if(!dbusconn.isConnected()){
		error = true;
		return;
	}

	obexService = "org.openobex";
	QString path = "/org/openobex";
	QString method = "CreateBluetoothServer";
	QString iface = "org.openobex.Manager";

	kDebug() << addr;

	manager = new QDBusInterface(obexService,path,iface,dbusconn);

	QList<QVariant> args;
	args << addr << pattern << require_pairing;
	kDebug() << args;
	kDebug() << "CallWithCallback";
	manager->callWithCallback( method, args, this, SLOT(serverCreated(QDBusObjectPath)), SLOT(serverCreatedError(QDBusError)) );
	delete dbus;
}

ObexServer::~ObexServer() 
{
	kDebug() << "Destructing obexServer :/";
	if(session)
		delete session;
	delete manager;
}


void ObexServer::serverCreated(QDBusObjectPath path)
{
	kDebug() << "obex server created";
	QDBusConnection* dbus = new QDBusConnection("dbus");
	QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");

	sessionPath = path.path();

	sessionIface = "org.openobex.Server";

	session = new QDBusInterface(obexService,sessionPath,sessionIface,dbusconn);
	
	//This interface MUST be valid too, if not is because openobex have some problem
	if(!session->isValid()){
		emit openObexError();
		return;
	}
	kDebug() << "session interface created for: " << sessionPath ;

// 	connect the DBus Signal to slots
	dbusconn.connect("",sessionPath,sessionIface,"Started",this,SLOT(slotStarted()));
	dbusconn.connect("",sessionPath,sessionIface,"Stopped",this,SLOT(slotStopped()));
	dbusconn.connect("",sessionPath,sessionIface,"Closed",this,SLOT(slotClosed()));
	dbusconn.connect("",sessionPath,sessionIface,"SessionCreated",this,SLOT(slotSessionCreated(QDBusObjectPath)));
	dbusconn.connect("",sessionPath,sessionIface,"SessionRemoved",this,SLOT(slotSessionRemoved(QDBusObjectPath)));
	dbusconn.connect("",sessionPath,sessionIface,"ErrorOccured",this,SLOT(slotErrorOccured(const QString&, const QString&)));

	delete dbus;
	emit serverReady();
}

void ObexServer::serverCreatedError(QDBusError err)
{
	kDebug() << "Error creating Bluetooth Server: " << err.message();
// 	KMessageBox::error(0 , err.message(), i18n("Error creating Bluetooth Server"));
}

void ObexServer::start(const QString& path, bool allow_write, bool auto_accept) {
	m_serverPath = path;
	session->call("Start", path, allow_write, auto_accept);
}

void ObexServer::stop() {
	kDebug() << "obex server stopped";
	//session->call(QDBus::BlockWithGui, "Stop");
	session->call("Stop");
}

void ObexServer::close() {
	kDebug() << "obex server closed";
	//session->call(QDBus::BlockWithGui, "Close");
	session->call("Close");
}

bool ObexServer::isStarted() {
	QDBusReply<bool> reply = session->call("IsStarted");
	if (reply.isValid())
		return reply.value();
	else
		return false;
}

QMap<QString,QString> ObexServer::getServerSessionInfo(QDBusObjectPath path) {
	qDBusRegisterMetaType<StringMapReply>();
	QList<QVariant> argumentList;
	argumentList << QVariant::fromValue(path);
	QDBusReply<QMap<QString, QString> > reply = session->callWithArgumentList(QDBus::Block, QLatin1String("GetServerSessionInfo"), argumentList);
	if (reply.isValid()) {
		return reply.value();
	} else {
		kDebug() << "replay not valid" << reply.error().message();
		return QMap<QString,QString>();
	}
}

QString ObexServer::path() {
	return m_serverPath;
}

//SLOTS to emit the signals
void ObexServer::slotStarted()
{
	kDebug() << "server started";
	emit started();
}

void ObexServer::slotStopped()
{
	kDebug() << "server stopped";
	emit stopped();
}

void ObexServer::slotClosed()
{
	kDebug() << "server closed";
	emit closed();
}

void ObexServer::slotSessionCreated(QDBusObjectPath path) {
	kDebug() << "server session created";
	emit sessionCreated(path.path());
}

void ObexServer::slotSessionRemoved(QDBusObjectPath path) {
	kDebug() << "server session removed";
	emit sessionRemoved(path.path());
}

void ObexServer::slotErrorOccured(const QString& error_name, const QString& error_message)
{
	kDebug() << "server error";
	emit errorOccured(error_name,error_message);
}