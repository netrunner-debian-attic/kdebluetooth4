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

#ifndef OBEXSERVER_H
#define OBEXSERVER_H
#include <QtDBus>
#include <QObject>

class ObexServer : public QObject
{
Q_OBJECT

public:
	ObexServer(QObject* parent, const QString& addr, const QString& pattern, bool require_pairing);
	~ObexServer();

	void start(const QString&, bool, bool);
	void stop();
	void close();
	bool isStarted();
	bool error;
	QMap<QString,QString> getServerSessionInfo(QDBusObjectPath);
	QString path();

private:
	QObject* m_parent;
	QDBusInterface* manager;
	QDBusInterface* session;
	QString sessionPath;	
	QString sessionIface;
	QString obexService;
	QString m_serverPath;

public slots:
	void serverCreated(QDBusObjectPath);
	void serverCreatedError(QDBusError);
	void slotStarted();
	void slotStopped();
	void slotClosed();
	void slotErrorOccured(const QString& error_name, const QString& error_message);
	void slotSessionCreated(QDBusObjectPath path);
	void slotSessionRemoved(QDBusObjectPath path);

signals:
	void serverReady();
	void started();
	void stopped();
	void closed();
	void errorOccured(const QString& error_name, const QString& error_message);
	void sessionCreated(const QString& path);
	void sessionRemoved(const QString& path);
	void openObexError();
};

#endif
