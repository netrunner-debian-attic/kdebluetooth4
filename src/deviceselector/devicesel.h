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

#ifndef DEVICESEL_H
#define DEVICESEL_H

#include <QtGui>
#include <QDBusObjectPath>
#include <KIcon>
#include <KDebug>
#include <klocalizedstring.h>
#include <solid/control/bluetoothmanager.h>

#include "ui_deviceselector.h"

class DeviceSel : public QDialog,
		  private Ui_deviceSelector
{

Q_OBJECT

public:
	DeviceSel(QObject* parent,QStringList filter);
	~DeviceSel();
	
	void setOkButtonText(const QString&);
	void setInfoLabel(const QString&);
	void hideExtraCheckBox();

private:
         QObject* m_parent;

	 Solid::Control::BluetoothInterface* adapter;

         QMap<QString, QMap< QString,QVariant > > macMap;
	 QListWidgetItem* item;
	 QStringList filterList;

         QPixmap normalPixmap;
         QPixmap flashPixmap;

         QTimer* logoTimer;


public slots:

	void slotCancel();
	void slotRemoteDeviceFound(const QString& , const QMap<QString,QVariant>& );
	void slotSend();
	void slotSearch();
	void slotUnlockChecker(bool);
        void slotDeviceSelected(QListWidgetItem*);

private slots:
        void setNormalLogo();
        void setFlashingLogo();

signals:
	void deviceSelected(const QString& mac);
	void selectorCancelled();
	void unlockChanged(bool);

};

#endif
