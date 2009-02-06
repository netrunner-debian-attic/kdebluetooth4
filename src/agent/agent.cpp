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


#include <qdebug.h>

#include "agent.h"

Agent::Agent( QApplication* application) : QDBusAbstractAdaptor(application), app(application)
{
    exitOnReleaseVal = false;

    Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();	
    adapter = new Solid::Control::BluetoothInterface(man.defaultInterface());

    passkeyDialog = new PasskeyDialog(this);
    authDialog = new AuthDialog(this);

    
    connect(passkeyDialog,SIGNAL(okClicked()),this,SLOT(slotOkClicked()));

    qDebug() << "AGENT registered !";
}

Agent::~Agent()
{
//   if (adapter)
//        delete adapter;
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

void Agent::Authorize(QDBusObjectPath device, const QString& uuid)
{
    qDebug() << "AGENT-Authorize " << device.path() << " Service: " << uuid;
    remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());
    authDialog->setName(remoteDevice->name());
    authDialog->setAddr(remoteDevice->address());
    authDialog->setService(uuid);
    int trust = authDialog->exec();
    qDebug() << "trust value = " << trust;

    switch(trust) {
        case(2): {
                remoteDevice->setTrusted(true);
                qDebug() << "Set Always Trust for " << remoteDevice->name();
                return;
        }
        case(1): {
            return;  
        }
        case(0): {
            //FIXME seems not to work
            QDBusMessage msg = QDBusMessage::createError("org.bluez.Agent","Authorization failed");
            QDBusConnection bus("systembus");
            bus.send(msg);
            break;
        }
    }
}

QString Agent::RequestPinCode(QDBusObjectPath device)
{
    qDebug() << "AGENT-RequestPinCode " << device.path();

    remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());

    passkeyDialog->setName(remoteDevice->name());
    passkeyDialog->setAddr(remoteDevice->address());
    passkeyDialog->clearPinCode();
    
    bool done = passkeyDialog->exec();

    qDebug() << "pinCode " << pincode;

    if (done)
            return pincode;
    else
            return 0;
}

quint32 Agent::RequestPasskey(QDBusObjectPath device)
{
    qDebug() << "AGENT-RequestPasskey " << device.path();

    remoteDevice = adapter->findBluetoothRemoteDeviceUBI(device.path());

    passkeyDialog->setName(remoteDevice->name());
    passkeyDialog->setAddr(remoteDevice->address());
    
    bool done = passkeyDialog->exec();

    qDebug() << "passkey " << QString::number(passkey);

    if (done)
            return passkey;
    else
            return 0;
}

void Agent::DisplayPasskey(QDBusObjectPath device, quint32 passkey)
{
    qDebug() << "AGENT-DisplayPasskey " << device.path() << ", " << QString::number(passkey);
}

void Agent::RequestConfirmation(QDBusObjectPath device, quint32 passkey)
{
    qDebug() << "AGENT-RequestConfirmation " << device.path() << ", " << QString::number(passkey);
    QString confirm;
    qDebug() << "Confirm Passkey (yes/no)";
    confirm = "yes";
    if (confirm == "yes")
        return;
    QDBusMessage msg = QDBusMessage::createError("org.bluez.Agent","Passkey doesn't match");
    QDBusConnection bus("systembus");
    bus.send(msg);
}

void Agent::ConfirmModeChange(const QString& mode)
{
    qDebug() << "AGENT-ConfirmModeChange " << mode;
}

void Agent::Cancel()
{
    qDebug() << "AGENT-Cancel";
}

void Agent::slotOkClicked()
{
	passkey = passkeyDialog->getPin().toUInt();
        pincode = passkeyDialog->getPin();
}
