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

#ifndef ADAPTERCONFIG_H
#define ADAPTERCONFIG_H

#include <KConfig>
#include <KConfigGroup>
#include <QtGui>
#include <QMap>
#include <QMainWindow>
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothinterface.h>

#include "adapterwidget.h"
#include "ui_adapterConf.h"


class AdapterConfig :   public QMainWindow,
			private Ui_AdapterUi
{
Q_OBJECT

public:
	AdapterConfig(QObject* parent = 0);
	~AdapterConfig();

	AdapterWidget* tab;
	
	void initialize();

private:
	QObject* m_parent;
	const Solid::Control::BluetoothManager& btmanager;
	
	void searchAdapters();
	void setAdapterWidgetProperties(AdapterWidget*,Solid::Control::BluetoothInterface);

	QMap<QString,AdapterWidget*>* adapterMap;

public slots:

	void slotQuit();
	void setTabName(AdapterWidget*, QString);
	void adapterAdded(const QString&);
	void adapterRemoved(const QString&);

};


#endif
