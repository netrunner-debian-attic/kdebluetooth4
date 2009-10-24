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


#ifndef DEVICEMAN_H
#define DEVICEMAN_H

#include "ui_deviceman.h"

#include <QMainWindow>
#include <QListWidget>
#include <QProgressDialog>
#include <QTableWidgetItem>

#include <KConfig>
#include <KIcon>
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothinterface.h>
#include <solid/control/bluetoothinputdevice.h>

class DeviceMan : public QMainWindow, private Ui_DeviceManager
{
Q_OBJECT

public:
	DeviceMan(QApplication* parent = 0);
	~DeviceMan();
	 
private:
	QApplication* m_parent;
	Solid::Control::BluetoothRemoteDeviceList devList;
	Solid::Control::BluetoothInterface* iface;
	void getConfiguredDevices();
	QTableWidgetItem* item;
	QMap<QString, Solid::Control::BluetoothRemoteDevice*>* remoteDeviceMap;
	QListWidgetItem* current;
	KIcon findIcon(QString, QString);

public slots:
	void slotQuitApp(bool);
	void slotStartWizard(bool);
	void slotShowDetails();
	void slotConnectDevice(bool);
	void slotDeviceAdded(const QString&);
	void slotDeviceRemoved(const QString&);
	void slotRemoveDevice(bool);
	void slotChangeTrust(bool);
	void slotMenuActive();
	void slotDevicePropertyChanged(const QString&, const QVariant&);
};
#endif
