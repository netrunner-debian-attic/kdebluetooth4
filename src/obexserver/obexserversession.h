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

#ifndef OBEXSERVERSESSION_H
#define OBEXSERVERSESSION_H

#include "obexserversessionfiletransfer.h"
#include <QtDBus>
#include <QObject>

class ObexServer;
class KJob;
class ObexServerSession : public QObject
{
Q_OBJECT

public:
	ObexServerSession(QObject*, ObexServer*, const QString&);
	~ObexServerSession();

	void accept();
	void reject();
	void disconnect();
	QMap<QString,QVariant> getTransferInfo();
	void cancel();

	QString path();
	ObexServer* server();

private:
	QObject* m_parent;
	QDBusInterface* manager;
	QDBusInterface* session;
	ObexServer* m_obexServer;

	QString obexService;
	QString m_path;
	ObexServerSessionFileTransfer *fileTransfer;

private slots:
	void slotCancelled();
	void slotDisconnected();
	void slotTransferStarted(const QString& filename, const QString& local_path, qulonglong total_bytes);
	void slotErrorOccurred(const QString& error_name, const QString& error_message);


signals:
	void cancelled();
	void disconnected();
	void transferStarted(KJob*);
	void errorOccurred(const QString&, const QString&);
};

#endif
