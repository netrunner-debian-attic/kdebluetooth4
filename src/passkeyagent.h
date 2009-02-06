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

#ifndef PASSKEY_H
#define PASSKEY_H

#include "passkeydialog.h"

#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothremotedevice.h>
#include <solid/control/bluetoothinterface.h>
#include <solid/control/bluetoothsecurity.h>


class PasskeyAgent : public Solid::Control::BluetoothPasskeyAgent
{
Q_OBJECT

public:
	PasskeyAgent(QObject* parent = 0);
	~PasskeyAgent();


private:
	QObject* m_parent;

	PasskeyDialog* dialog;
	
	Solid::Control::BluetoothInterface* iface;

	QString passkey;

public slots:

	virtual QString requestPasskey(const QString &ubi, bool isNumeric);
	virtual bool confirmPasskey(const QString &ubi, const QString &passkey);
	virtual void displayPasskey(const QString &ubi, const QString &passkey);
	virtual void keypress(const QString &ubi);
	virtual void completedAuthentication(const QString &ubi);
	virtual void cancelAuthentication(const QString &ubi);

	void slotOkClicked();
	
};


#endif
