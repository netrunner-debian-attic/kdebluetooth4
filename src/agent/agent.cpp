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

#include "agent.h"

#include <QDebug>

Agent::Agent( QApplication* application, Solid::Control::BluetoothInterface* iface) : QDBusAbstractAdaptor(application), app(application), adapter(iface)
{
	exitOnReleaseVal = false;

	passkeyDialog = new PasskeyDialog(this);
	authDialog = new AuthDialog(this);
	confirmDialog = new ConfirmDialog(this);

	connect(passkeyDialog,SIGNAL(okClicked()),this,SLOT(slotOkClicked()));

	qDebug() << "AGENT registered !";
}

Agent::~Agent()
{
	delete authDialog;
	delete passkeyDialog;
	delete confirmDialog;
	qDebug() << "Agent deleted";
}

void Agent::setBluetoothInterface(Solid::Control::BluetoothInterface* iface)
{
	adapter = iface;
}

bool Agent::exitOnRelease()
{
	return exitOnReleaseVal;
}

void Agent::setExitOnRelease(bool val)
{
	exitOnReleaseVal = val;
}

void Agent::Release()
{
	qDebug() << "Agent Release";
	if ( exitOnRelease() )
		app->quit();
}

int Agent::execDialog(QDialog* dialog)
{
	int ret;

	currentDialog = dialog;
	ret = currentDialog->exec();

	currentDialog = 0;
	return ret;
}

void Agent::Authorize(QDBusObjectPath device, const QString& uuid, const QDBusMessage &msg)
{
	qDebug() << "AGENT-Authorize " << device.path() << " Service: " << uuid;
	remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());
	authDialog->setName(remoteDevice->name());
	authDialog->setAddr(remoteDevice->address());
	authDialog->setService(uuid);

	int trust = execDialog(authDialog);
	qDebug() << "trust value = " << trust;

	switch(trust) {
		case(2): {
			remoteDevice->setTrusted(true);
			qDebug() << "Set Always Trust for " << remoteDevice->name();
			return;
		}
		case(1): {
			QDBusMessage error = msg.createErrorReply("org.bluez.Error.Rejected", "Authorization rejected");
			QDBusConnection::systemBus().send(error);
			return;
		}
		case(0): {
			QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Authorization canceled");
			QDBusConnection::systemBus().send(error);
			break;
		}
	}
}

QString Agent::RequestPinCode(QDBusObjectPath device, const QDBusMessage &msg)
{
	qDebug() << "AGENT-RequestPinCode " << device.path();

	remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());

	passkeyDialog->setName(remoteDevice->name());
	passkeyDialog->setAddr(remoteDevice->address());
	passkeyDialog->clearPinCode();

	bool done = execDialog(passkeyDialog);

	qDebug() << "pinCode " << pincode;

	if (done)
		return pincode;

	QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Pincode request failed");
	QDBusConnection::systemBus().send(error);
	return 0;
}

quint32 Agent::RequestPasskey(QDBusObjectPath device, const QDBusMessage &msg)
{
	qDebug() << "AGENT-RequestPasskey " << device.path();

	remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());

	passkeyDialog->setName(remoteDevice->name());
	passkeyDialog->setAddr(remoteDevice->address());

	bool done = execDialog(passkeyDialog);

	qDebug() << "passkey " << QString::number(passkey);

	if (done)
		return passkey;

	QDBusMessage error = msg.createErrorReply("org.bluez.Error.Canceled", "Passkey request failed");
	QDBusConnection::systemBus().send(error);
	return 0;
}

void Agent::DisplayPasskey(QDBusObjectPath device, quint32 passkey)
{
	qDebug() << "AGENT-DisplayPasskey " << device.path() << ", " << QString::number(passkey);
}

void Agent::RequestConfirmation(QDBusObjectPath device, quint32 passkey, const QDBusMessage &msg)
{
	qDebug() << "AGENT-RequestConfirmation " << device.path() << ", " << QString::number(passkey);
	remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());
	confirmDialog->setName(remoteDevice->name());
	confirmDialog->setAddr(remoteDevice->address());
	confirmDialog->setPassKey(QString::number(passkey));

	bool confirm = execDialog(confirmDialog);

	if (confirm)
		return;

	QDBusMessage error = msg.createErrorReply("org.bluez.Error.Rejected", "Confirmation rejected");
	QDBusConnection::systemBus().send(error);
}

void Agent::ConfirmModeChange(const QString& mode, const QDBusMessage &msg)
{
	qDebug() << "AGENT-ConfirmModeChange " << adapter->name() << " " << adapter->address() << " " << mode;
	confirmDialog->setName(adapter->name());
	confirmDialog->setAddr(adapter->address());
	confirmDialog->setMode(mode);

	bool confirm = execDialog(confirmDialog);

	if (confirm)
		return;

	QDBusMessage error = msg.createErrorReply("org.bluez.Error.Rejected", "Mode change rejected");
	QDBusConnection::systemBus().send(error);
}

void Agent::Cancel()
{
	qDebug() << "AGENT-Cancel";

	if (!currentDialog)
		return;

	currentDialog->reject();
	currentDialog = 0;
}

void Agent::slotOkClicked()
{
	passkey = passkeyDialog->getPin().toUInt();
	pincode = passkeyDialog->getPin();
}