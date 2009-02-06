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

#ifndef OBEXSESSION_H
#define OBEXSESSION_H

class ObexSession : public QObject
{
Q_OBJECT

public:
	ObexSession(QObject* parent, const QString& srcAddr, const QString& addr, const QString& service);
	~ObexSession();

	void connect();
	void disconnect();
	void close();
	bool isConnected();
	void changeCurrentFolder(const QString& path);
	void changeCurrentFolderBackward();
	void changeCurrentFolderToRoot();
	QString getCurrentPath();
	void copyRemoteFile(const QString& remote_filename, const QString& local_path);
	void createFolder(const QString& folder_name);
	QMap<QString,QVariant> retrieveFolderListing();
	QMap<QString,QVariant> getCapability();
	void sendFile(const QString& local_path);
	void deleteRemoteFile(const QString& remote_filename);
	QMap<QString,QVariant> getTransferInfo();
	bool isBusy();
	void cancel();
	


private:
        QObject* m_parent;
	QDBusInterface* manager;
	QDBusInterface* session;

	QString sessionPath;	
	QString sessionIface;
	QString obexService;

public slots:

	void sessionCreated(QDBusObjectPath);
	void sessionCreatedError(QDBusError);

	void slotCancelled();
	void slotConnected();
	void slotDisconnected();
	void slotClosed();
	void slotTransferStarted(const QString& filename, const QString& local_path, qulonglong total_bytes);
	void slotTransferProgress(qulonglong bytes_transferred);
	void slotTransferCompleted();
	void slotErrorOccurred(const QString& error_name, const QString& error_message);


signals:

	void cancelled();
	void connected();
	void disconnected();
	void closed();
	void transferStarted(const QString& filename, const QString& local_path, qulonglong total_bytes);
	void transferProgress(qulonglong bytes_transferred);
	void transferCompleted();
	void errorOccurred(const QString& error_name, const QString& error_message);
	

};

#endif
