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

#include "obexserversessionfiletransfer.h"
#include "obexserversession.h"
#include "obexserver.h"

#include <QTimer>
#include <qdbusconnection.h>
#include <qdbusinterface.h>

#include <KDebug>
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothremotedevice.h>


ObexServerSessionFileTransfer::ObexServerSessionFileTransfer(ObexServerSession* serverSession, QDBusInterface* session, const QString& filename, const QString& path, qulonglong size) {
	setAutoDelete(false);
	m_serverSession = serverSession;
	m_dbusSession = session;
	m_fileName = filename;
	if(path.length() != 0)
		m_localPath = path;
	else
		m_localPath = m_serverSession->server()->path() + "/" + filename;
	m_totalFileSize = size;
	setCapabilities(Killable);

	kDebug() << m_serverSession->path();
	QMap<QString, QString> sessionInfo = m_serverSession->server()->getServerSessionInfo(QDBusObjectPath(m_serverSession->path()));
	kDebug() << sessionInfo["BluetoothAddress"];
	Solid::Control::BluetoothManager &bluetoothManager = Solid::Control::BluetoothManager::self();
	Solid::Control::BluetoothInterface* bluetoothInterface = new Solid::Control::BluetoothInterface(bluetoothManager.defaultInterface());
	bluetoothDevice = bluetoothInterface->findBluetoothRemoteDeviceAddr(sessionInfo["BluetoothAddress"]);

	m_remoteName =  bluetoothDevice.name();
	m_remoteAddr = bluetoothDevice.address();
}


ObexServerSessionFileTransfer::~ObexServerSessionFileTransfer() {
}

void ObexServerSessionFileTransfer::start() {
	QTimer::singleShot(1200, this, SLOT(receiveFiles()));
}

void ObexServerSessionFileTransfer::reject() {
	m_dbusSession->call("Reject");
	doKill();
}

void ObexServerSessionFileTransfer::receiveFiles() {
	emit description(this, "Receiving file over bluetooth", QPair<QString, QString>("From", bluetoothDevice.name()), QPair<QString, QString>("To", m_localPath));

	QDBusConnection dbusConnection = m_dbusSession->connection();
	dbusConnection.connect("", "", m_dbusSession->interface(), "TransferProgress", this, SLOT(slotTransferProgress(qulonglong)));
	dbusConnection.connect("", "", m_dbusSession->interface(), "TransferCompleted", this, SLOT(slotTransferCompleted()));
	dbusConnection.connect("", "", m_dbusSession->interface(), "ErrorOccurred", this, SLOT(slotErrorOccured(QString,QString)));
	kDebug() << "Transfer started ...";
	setTotalAmount(Bytes, m_totalFileSize);
	setProcessedAmount(Bytes, 0);
	m_dbusSession->call("Accept");
}

void ObexServerSessionFileTransfer::slotTransferProgress(qulonglong transferred) {
	kDebug() << "Transfer progress ...";
	ulong currentPercent = percent();
	setProcessedAmount(Bytes, transferred);
	ulong newPercent = percent();
	double unit = m_totalFileSize / 100.0;
	emitSpeed(newPercent*unit - currentPercent*unit);
}

void ObexServerSessionFileTransfer::slotTransferCompleted() {
	kDebug() << "Transfer completed";
	setProcessedAmount(Bytes, m_totalFileSize);
	emitResult();
}

void ObexServerSessionFileTransfer::slotErrorOccured(const QString& reason1, const QString& reason2) {
	kDebug() << "Transfer disconnected";
	kDebug() << reason1;
	kDebug() << reason2;
	#warning Add setErrorText after 0.4 release.
	emitResult();
}

void ObexServerSessionFileTransfer::feedbackReceived()
{
	emitResult();
}

bool ObexServerSessionFileTransfer::doKill() {
	kDebug() << "doKill called";
	m_serverSession->disconnect();
	m_dbusSession->call("Cancel");
	return true;
}

QString ObexServerSessionFileTransfer::fileName() {
	return m_fileName;
}

QString ObexServerSessionFileTransfer::localPath() {
	return m_localPath;
}

QString ObexServerSessionFileTransfer::remoteName() const{
	return m_remoteName;
}

QString ObexServerSessionFileTransfer::remoteAddr() const{
	return m_remoteAddr;
}

void ObexServerSessionFileTransfer::setLocalPath(const QString& newPath) {
	m_localPath = newPath;
}

qulonglong ObexServerSessionFileTransfer::fileSize() {
	return m_totalFileSize;
}
