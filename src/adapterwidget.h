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

#ifndef ADAPTERWIDGET_H
#define ADAPTERWIDGET_H

#include "ui_adapterWidget.h"

#include <QWidget>

#include <KConfig>
#include <solid/control/bluetoothinterface.h>

class AdapterWidget :   public QWidget, private Ui_AdapterWidget
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
	void slotModeChanged(int);
	void setDiscoverableTimeout(int);
	void setName(QString);
	void slotNameChanged();

signals:
	void nameFieldChanged(AdapterWidget*, QString);
};
#endif