/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <QObject>
#include <QString>
#include <QMap>
#include <solid/control/bluetoothinterface.h>

#ifndef KBLUELOCK_H
#define KBLUELOCK_H

class KBlueLock : public QObject
{
Q_OBJECT

public:
	KBlueLock(Solid::Control::BluetoothInterface*);
	~KBlueLock();
	
	bool enabled();
	
	QString lockDevice();
	
public slots:
	void enable();
	void enable(const QString&);
	void disable();
	void unlockEnable(bool);
	
private:
	QString m_lockDevice;
	Solid::Control::BluetoothInterface* m_adapter;
	bool m_lockEnabled;
	bool m_unlockEnabled;
	KConfig* config;
	
private slots:
	void slotTargetDeviceFound(const QString&, const QMap<QString, QVariant>&);
	void slotTargetDeviceBack(const QString&, const QMap<QString, QVariant>&);
	void slotValidateLockDevice(const QString&);
	
signals:
  	void lockEnabled();
	void lockDisabled();
	void lockReady();
};

#endif // KBLUELOCK_H
