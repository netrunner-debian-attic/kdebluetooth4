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


#ifndef AGENT_H
#define AGENT_H

#include "authdialog.h"
#include "passkeydialog.h"
#include "confirmdialog.h"

#include <QtDBus>
#include <QApplication>
#include <QDebug>

#include <solid/control/bluetoothinterface.h>
#include <solid/control/bluetoothremotedevice.h>

class Agent : public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent")
//  Q_PROPERTY(bool exitOnRelease READ exitOnRelease WRITE setExitOnRelease)

public:
	Agent(QApplication* application,Solid::Control::BluetoothInterface*);
	~Agent();

	bool exitOnRelease();
	void setExitOnRelease(bool val);
	void setBluetoothInterface(Solid::Control::BluetoothInterface* iface);
	int execDialog(QDialog*);

public slots:
	void Release();
	void Authorize(QDBusObjectPath device, const QString& uuid, const QDBusMessage &msg);
	QString RequestPinCode(QDBusObjectPath device, const QDBusMessage &msg);
	quint32 RequestPasskey(QDBusObjectPath device, const QDBusMessage &msg);
	void DisplayPasskey(QDBusObjectPath device, quint32 passkey);
	void RequestConfirmation(QDBusObjectPath device, quint32 passkey, const QDBusMessage &msg);
	void ConfirmModeChange(const QString& mode, const QDBusMessage &msg);
	void Cancel();

private:
	QApplication* app;
	bool exitOnReleaseVal;
	Solid::Control::BluetoothInterface* adapter;
	Solid::Control::BluetoothRemoteDevice* remoteDevice;
	QString agentPath;
	quint32 passkey;
	QString pincode;

	PasskeyDialog* passkeyDialog;
	AuthDialog* authDialog;
	ConfirmDialog* confirmDialog;
	QDialog* currentDialog;

private slots:
    void slotOkClicked();
};
#endif
