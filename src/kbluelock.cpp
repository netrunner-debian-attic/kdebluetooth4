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

#include "kbluelock.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QFile>
#include <QDir>

#include <KDebug>
#include <KConfig>
#include <KConfigGroup>

KBlueLock::KBlueLock(Solid::Control::BluetoothInterface* adapter)
{
	config = new KConfig("kbluetoothrc");

	m_lockDevice = config->group("KBlueLock").readEntry("Device","");
	m_adapter = adapter;
	m_lockEnabled = false;
	m_unlockEnabled = config->group("KBlueLock").readEntry("UnlockEnabled",true);
}

KBlueLock::~KBlueLock()
{
	delete config;
}

void KBlueLock::enable()
{
	enable(m_lockDevice);
}

void KBlueLock::enable(const QString& device)
{
	m_lockDevice = device;
	config->group("KBlueLock").writeEntry("Device",device);
	config->group("KBlueLock").sync();
	connect(m_adapter, SIGNAL(deviceFound(QString, QMap<QString, QVariant>)), this, SLOT(slotTargetDeviceFound(QString, QMap<QString, QVariant>)));
	connect(m_adapter, SIGNAL(deviceDisappeared(QString)), this, SLOT(slotValidateLockDevice(QString)));
	m_adapter->startDiscovery();
	kDebug() << "KBlueLock enabled";
	emit lockEnabled();
}

void KBlueLock::disable()
{
	m_adapter->stopDiscovery();
	m_lockEnabled = false;
	config->group("KBlueLock").writeEntry("LockEnabled",false);
	config->group("KBlueLock").sync();
	disconnect(m_adapter, SIGNAL(deviceFound(QString, QMap<QString, QVariant>)), this, SLOT(slotTargetDeviceFound(QString, QMap<QString, QVariant>)));
	disconnect(m_adapter, SIGNAL(deviceDisappeared(QString)), this, SLOT(slotValidateLockDevice(QString)));
	kDebug() << "KBlueLock disabled";
	emit lockDisabled();
}

void KBlueLock::unlockEnable(bool value)
{
	m_unlockEnabled = value;	
	config->group("KBlueLock").writeEntry("UnlockEnabled",value);
	config->group("KBlueLock").sync();
}

bool KBlueLock::enabled()
{
	return m_lockEnabled;
}

QString KBlueLock::lockDevice()
{
	return m_lockDevice;
}

void KBlueLock::slotTargetDeviceFound(const QString& address, const QMap< QString, QVariant >& )
{
	if (!m_lockEnabled && address == m_lockDevice) {
		kDebug() << "Target device found - Kbluelock ready";
		//lockDeviceName = props["Name"].toString();
		//showMessage("KBluetooth", "kbluelock is enabled now", "kbluetooth4");
		m_lockEnabled = true;
		config->group("KBlueLock").writeEntry("LockEnabled",true);
		config->group("KBlueLock").sync();
		disconnect(m_adapter, SIGNAL(deviceFound(QString, QMap<QString, QVariant>)), this, SLOT(slotTargetDeviceFound(QString, QMap<QString, QVariant>)));
		emit lockReady();
	}
}

void KBlueLock::slotValidateLockDevice(const QString& address)
{
	if (address == m_lockDevice) {		
		kDebug() << "KBlueLock armed -- Lock Device disappeared";
		//QDBusConnection dbusConn = QDBusConnection::connectToBus(QDBusConnection::SessionBus, "dbus");
		QString service = "org.freedesktop.ScreenSaver";
		QString path = "/ScreenSaver";
		QString method = "Lock";
		QString iface = "org.freedesktop.ScreenSaver";

		QDBusInterface interface(service, path, iface);
		interface.call(method);

		/*QFile file(QDir::homePath() + "/kbluelock.log");
	 	if (!file.open(QIODevice::Append | QIODevice::Text))
         		kDebug() << "cant open logfile";
	 	else {
			QTextStream out(&file);
	 	 	awaytime = QDateTime::currentDateTime();
			out << "Away from Computer: " << awaytime.toString() << "\n";
			file.flush();	 	 	
			file.close();
	 	}*/
		connect(m_adapter, SIGNAL(deviceFound(QString, QMap<QString, QVariant>)), this, SLOT(slotTargetDeviceBack(QString, QMap<QString, QVariant>)));
	}
}

void KBlueLock::slotTargetDeviceBack(const QString& address, const QMap< QString, QVariant >& )
{
	if (m_lockEnabled && m_unlockEnabled && address == m_lockDevice) {
		kDebug() << "Target device is back. Disabling Screensaver.";

		QString service = "org.freedesktop.ScreenSaver";
		QString path = "/ScreenSaver";
		QString method = "SetActive";
		QString iface = "org.freedesktop.ScreenSaver";

		QDBusInterface interface(service, path, iface);
		interface.call(method, false);

		/*QFile file(QDir::homePath() + "/.kbluetooth4/kbluelock.log");
		if (!file.open(QIODevice::Append | QIODevice::Text))
         	 	 kDebug() << "cant open logfile";
	 	else {
			QTextStream out(&file);

			out << "Back on Computer: " << QDateTime::currentDateTime ().toString() << "\n";
	 	 	out << "---------------------------------------------\n";
	 	    	out << ">>> Pause: " << awaytime.secsTo(QDateTime::currentDateTime())/60 << " min ";
	 	    	out << awaytime.secsTo(QDateTime::currentDateTime())%60 << " sec\n\n";
			file.flush();	 	 	
			file.close();
	 	}*/
	
		disconnect(m_adapter, SIGNAL(deviceFound(QString, QMap<QString, QVariant>)), this, SLOT(slotTargetDeviceBack(QString, QMap<QString, QVariant>)));
	}
}