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


#ifndef AGENT_H
#define AGENT_H



#include <QtDBus>
#include <QApplication>
#include <QDebug>
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothinterface.h>

#include "authdialog.h"
#include "passkeydialog.h"


class Agent : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Agent")

//  Q_PROPERTY(bool exitOnRelease READ exitOnRelease WRITE setExitOnRelease)

public:

    Agent(QApplication* application);
    ~Agent();

    bool exitOnRelease();
    void setExitOnRelease(bool val);

public slots:

    void Release();
    void Authorize(QDBusObjectPath device, const QString& uuid);
    QString RequestPinCode(QDBusObjectPath device);
    quint32 RequestPasskey(QDBusObjectPath device);
    void DisplayPasskey(QDBusObjectPath device, quint32 passkey);
    void RequestConfirmation(QDBusObjectPath device, quint32 passkey);
    void ConfirmModeChange(const QString& mode);
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

private slots:
    void slotOkClicked();

};

#endif
