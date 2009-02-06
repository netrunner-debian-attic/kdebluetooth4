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

#ifndef ADAPTERWIDGET_H
#define ADAPTERWIDGET_H

#include <KConfig>
#include <KConfigGroup>
#include <QtGui>
#include <QWidget>
#include <solid/control/bluetoothinterface.h>

#include "ui_adapterWidget.h"


class AdapterWidget :   public QWidget,
			private Ui_AdapterWidget
{
Q_OBJECT

public:
	 AdapterWidget(QString,QWidget* parent = 0);
	 ~AdapterWidget();
	
	void setMac(QString);
	void setDetails(QString,QString);

	void setDiscoverable(bool);

	QString getMac();
	QString getVersion();
	QString getName();
	QString getMode();
	int getDiscoverableTimeout();

	QString m_ubi;

private:
	Solid::Control::BluetoothInterface* iface;
	QString m_mac;
	QString m_version;
	QString m_name;

public slots:

	void slotSliderMoved(int);
	void slotModeChanged(const QString&);
	void setDiscoverableTimeout(int);
	void setName(QString);
	void slotNameChanged();

signals:
	
	void nameFieldChanged(AdapterWidget*, QString);
	

};


#endif
