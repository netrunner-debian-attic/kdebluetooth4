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

#include <KDebug>
#include "authagent.h"


AuthAgent::AuthAgent(QObject* parent) : Solid::Control::BluetoothAuthorizationAgent(parent)
{
	m_parent = parent;
	dialog = new AuthDialog(this);
	adapter = 0;
	
}

AuthAgent::~AuthAgent()
{
	if (adapter)
		delete adapter;
	delete dialog;

}

bool AuthAgent::authorize(const QString &localUbi,const QString &remoteAddress,const QString& serviceUuid)
{
	kDebug() << " authorize me! Service: " << serviceUuid;
	Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();
	adapter = new Solid::Control::BluetoothInterface(man.defaultInterface());
	QString remoteName = adapter->getRemoteName(remoteAddress);
	dialog->setName(remoteName);
	dialog->setAddr(remoteAddress);
	dialog->setService(serviceUuid);
	int trust = dialog->exec();
	kDebug() << "trust value = " << trust;

	if (trust == 2) {
		adapter->setTrusted(remoteAddress);
		kDebug() << "Always Trust " << remoteAddress;
	}
	
	return trust;
	

}

void AuthAgent::cancel(const QString &localUbi,const QString &remoteAddress,const QString& serviceUuid)
{
	kDebug() << "Authorization cancelled";
}
