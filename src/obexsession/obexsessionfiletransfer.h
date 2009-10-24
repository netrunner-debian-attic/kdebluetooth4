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

#ifndef OBEXSESSIONFILETRANSFER_H
#define OBEXSESSIONFILETRANSFER_H

#include <KJob>

class ObexSession;
class QDBusInterface;

class ObexSessionFileTransfer : public KJob
{
Q_OBJECT
public:
	ObexSessionFileTransfer(ObexSession*, QDBusInterface*, const QString&);
	~ObexSessionFileTransfer();

	void start();

protected:
	bool doKill();

private slots:
	void sendFile();
	void slotTransferStarted(const QString&, const QString&, qulonglong);
	void slotTransferProgress(qulonglong transferred);
	void slotTransferCompleted();

private:
	ObexSession* m_obexSession;
	QDBusInterface* m_dbusSession;
	QString m_localPath;
	qulonglong m_totalFileSize;
};
#endif // OBEXSESSIONFILETRANSFER_H
