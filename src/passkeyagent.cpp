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

#include "passkeyagent.h"
#include <KDebug>

PasskeyAgent::PasskeyAgent(QObject* parent) : Solid::Control::BluetoothPasskeyAgent(parent)
{

	m_parent = parent;
	dialog = new PasskeyDialog(this);
	Solid::Control::BluetoothManager &btmanager = Solid::Control::BluetoothManager::self();	
	iface = new Solid::Control::BluetoothInterface(btmanager.defaultInterface());

	connect(dialog,SIGNAL(okClicked()),this,SLOT(slotOkClicked()));

}

PasskeyAgent::~PasskeyAgent()
{

}



QString PasskeyAgent::requestPasskey(const QString &ubi, bool isNumeric)
{
	kDebug() << " passkey request from " << ubi;
	QString remoteName = iface->getRemoteName(ubi);

	dialog->setName(remoteName);
	dialog->setAddr(ubi);
	
	bool done = dialog->exec();

	kDebug() << "passkey" << passkey;

	if (done)
		return passkey;
	else
		return "";
}

bool PasskeyAgent::confirmPasskey(const QString &ubi, const QString &passkey)
{

return false;
}

void PasskeyAgent::displayPasskey(const QString &ubi, const QString &passkey)
{

}

void PasskeyAgent::keypress(const QString &ubi)
{

}

void PasskeyAgent::completedAuthentication(const QString &ubi)
{
	
}

void PasskeyAgent::cancelAuthentication(const QString &ubi)
{
}

void PasskeyAgent::slotOkClicked()
{
	passkey = dialog->getPin();
}



