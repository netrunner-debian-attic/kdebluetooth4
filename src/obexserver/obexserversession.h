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

#include <QtDBus>
#include <QtGui>

#include <QObject>

#ifndef OBEXSERVERSESSION_H
#define OBEXSERVERSESSION_H

class ObexServerSession : public QObject
{
Q_OBJECT

public:
	ObexServerSession(QObject* ,const QString&);
	~ObexServerSession();

	void accept();
	void reject();
	void disconnect();
	QMap<QString,QVariant> getTransferInfo();
	void cancel();


private:
        QObject* m_parent;
	QDBusInterface* manager;
	QDBusInterface* session;

	QString sessionPath;	
	QString sessionIface;
	QString obexService;

public slots:

	void slotCancelled();
	void slotDisconnected();
	void slotTransferStarted(const QString& filename, const QString& local_path, qulonglong total_bytes);
	void slotTransferProgress(qulonglong bytes_transferred);
	void slotTransferCompleted();
	void slotErrorOccurred(const QString& error_name, const QString& error_message);


signals:

	void cancelled();
	void disconnected();
	void transferStarted(const QString&, const QString&, qulonglong);
	void transferProgress(qulonglong);
	void transferCompleted();
	void errorOccurred(const QString&, const QString&);
	

};

#endif
