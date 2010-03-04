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

#include "obexsessionfiletransfer.h"
#include "obexsession.h"

#include <QTimer>
#include <KIO/JobUiDelegate>

#include <KDebug>
#include <solid/control/bluetoothinterface.h>
#include <solid/control/bluetoothmanager.h>

ObexSessionFileTransfer::ObexSessionFileTransfer(ObexSession* osession, QDBusInterface* dsession, const QString& path) {
	m_obexSession = osession;
	m_dbusSession = dsession;
	m_localPath = path;
	setCapabilities(Killable);
}

ObexSessionFileTransfer::~ObexSessionFileTransfer() {

}

bool ObexSessionFileTransfer::doKill() {
	kDebug() << "Transfer canceled ...";
	m_dbusSession->call("Cancel");
	return true;
}

void ObexSessionFileTransfer::start() {
	QTimer::singleShot(0, this, SLOT(sendFile()));
}

void ObexSessionFileTransfer::sendFile() {
	Solid::Control::BluetoothManager &bluetoothManager = Solid::Control::BluetoothManager::self();
	Solid::Control::BluetoothInterface* bluetoothInterface = new Solid::Control::BluetoothInterface(bluetoothManager.defaultInterface());
	Solid::Control::BluetoothRemoteDevice bluetoothDevice = bluetoothInterface->findBluetoothRemoteDeviceAddr(m_obexSession->targetAddress());

	emit description(this, "Sending file over bluetooth", QPair<QString, QString>("From", m_localPath), QPair<QString, QString>("To", bluetoothDevice.name()));

	QDBusConnection dbusConnection = m_dbusSession->connection();
	dbusConnection.connect(m_dbusSession->service(), m_dbusSession->path(), m_dbusSession->interface(), "TransferStarted", this, SLOT( slotTransferStarted(const QString&, const QString&, qulonglong) ));
	dbusConnection.connect(m_dbusSession->service(), m_dbusSession->path(), m_dbusSession->interface(), "TransferProgress", this, SLOT( slotTransferProgress(qulonglong) ));
	dbusConnection.connect(m_dbusSession->service(), m_dbusSession->path(), m_dbusSession->interface(), "TransferCompleted", this, SLOT( slotTransferCompleted() ));
	
	m_dbusSession->call("SendFile", m_localPath);
}

void ObexSessionFileTransfer::slotTransferStarted(const QString& filename, const QString& path, qulonglong totalSize) {
	Q_UNUSED(filename);
	Q_UNUSED(path);

	kDebug() << "Transfer started ...";
	setTotalAmount(Bytes, totalSize);
	m_totalFileSize = totalSize;
	setProcessedAmount(Bytes, 0);
	m_time = QTime::currentTime();
	m_procesedBytes = 0;
}

void ObexSessionFileTransfer::slotTransferProgress(qulonglong transferred) {
	kDebug() << "Transfer progress ..." << transferred;

	QTime currentTime = QTime::currentTime();
	int time = m_time.secsTo(currentTime);
	if (time != 0) {
		qulonglong diffBytes = transferred - m_procesedBytes;
		float speed = diffBytes / time;
		kDebug() << "Bytes: " << diffBytes << " Speed: " << speed;
		emitSpeed(speed);
		m_time = currentTime;
		m_procesedBytes = transferred;
	}
	setProcessedAmount(Bytes, transferred);
}

void ObexSessionFileTransfer::slotTransferCompleted() {
	kDebug() << "Transfer completed";
	setProcessedAmount(Bytes, m_totalFileSize);
	emitResult();
}