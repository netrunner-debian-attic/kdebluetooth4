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


#include "devicemanager.h"
#include "ui_deviceman.h"

#include <QList>

#include <KDebug>
#include <KApplication>
#include <KProcess>
#include <kjob.h>

#define HID "00001124-0000-1000-8000-00805F9B34FB"
#define FTP "00001106-0000-1000-8000-00805F9B34FB"

DeviceMan::DeviceMan(QApplication* parent) : Ui_DeviceManager(), m_parent(parent)
{
	Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();
	if (man.bluetoothInterfaces().size() > 0) {
		iface = new Solid::Control::BluetoothInterface(man.defaultInterface());

		connect(iface,SIGNAL(deviceCreated(const QString&)),this,SLOT(slotDeviceAdded(const QString&)));
		connect(iface,SIGNAL(deviceRemoved(const QString&)),this,SLOT(slotDeviceRemoved(const QString&)));

		kDebug() << "Device Manager - Welcome";
//	 setParent(kapp);
		setupUi(this);
		setWindowIcon(KIcon("kbluetooth"));

		connect(newDevButton,SIGNAL(clicked(bool)),this,SLOT(slotStartWizard(bool)));
		connect(removeButton,SIGNAL(clicked(bool)),this,SLOT(slotRemoveDevice(bool)));
		connect(connectButton,SIGNAL(clicked(bool)),this,SLOT(slotConnectDevice(bool)));
		connect(setTrustButton,SIGNAL(clicked(bool)),this,SLOT(slotChangeTrust(bool)));

		connect(deviceListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotShowDetails()));

		remoteDeviceMap = new QMap<QString,Solid::Control::BluetoothRemoteDevice*>;
		show();
		connectButton->setEnabled(false);
		removeButton->setEnabled(false);
		setTrustButton->setEnabled(false);
		removeButton->setEnabled(false);
		
		connectButton->setIcon(KIcon("applications-internet"));
		removeButton->setIcon(KIcon("user-trash"));
		newDevButton->setIcon(KIcon("list-add"));
		setTrustButton->setIcon(KIcon("dialog-password"));
		
		current = 0;
		
		getConfiguredDevices();
	} else {
		kDebug() << "No Bluetooth Adapter found";
		close();
	}
}

DeviceMan::~DeviceMan()
{
	kDebug() << "inputDeviceMap deleted";
	if ( remoteDeviceMap->size() > 0 ) {
		qDeleteAll (remoteDeviceMap->begin(),remoteDeviceMap->end());
		remoteDeviceMap->clear();
	}
	if (remoteDeviceMap)
//	        delete remoteDeviceMap;
	kDebug() << "remoteDeviceMap deleted";
	if (iface)
//		delete iface;
	kDebug() << "Device Manager closed";
}

void DeviceMan::slotQuitApp(bool /*set*/)
{
	close();
}

void DeviceMan::slotStartWizard(bool /*set*/)
{
	kDebug() << "Starting Wizard";
	KProcess process;
	process.setProgram("kbluetooth-inputwizard");
	process.execute();
}

void DeviceMan::getConfiguredDevices()
{
	devList = iface->listDevices();
	kDebug() << "Device List Size: " << devList.size();
	if (devList.size() > 0) {
		Solid::Control::BluetoothRemoteDevice* dev;
		foreach(dev,devList) {
			kDebug() << dev->name();
			QStringList uuids = dev->uuids();
			deviceListWidget->addItem(dev->name());
			Solid::Control::BluetoothRemoteDevice* rem = iface->findBluetoothRemoteDeviceUBI(dev->ubi());
			remoteDeviceMap->insert(dev->address(),rem);
		}

		Solid::Control::BluetoothRemoteDevice* remDev;
		foreach(remDev, *remoteDeviceMap) {
			connect(remDev,SIGNAL(propertyChanged(const QString&, const QVariant&)),this,SLOT(slotDevicePropertyChanged(const QString&, const QVariant&)));
		}
		deviceListWidget->setCurrentRow(0);
		//slotShowDetails(deviceListWidget->item(0));
	}
	//connect(deviceListWidget, SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(slotShowDetails(QListWidgetItem*)));
}

void DeviceMan::slotShowDetails()
{
	current = deviceListWidget->currentItem();
	kDebug() << "Current show details" << current;
	if(deviceListWidget->selectedItems().size() == 0) {
		current = 0;
	}
	statusbar->clearMessage();
	if (current) {
		connectButton->setEnabled(true);
		actionRemove->setEnabled(true);
		setTrustButton->setEnabled(true);
		removeButton->setEnabled(true);

		Solid::Control::BluetoothRemoteDevice* dev = 0;
		foreach(dev, *remoteDeviceMap) {
			if (current->text() == dev->name())
				break;
		}

//		Solid::Control::BluetoothRemoteDevice* remoteDev = remoteDeviceMap->value(dev.address());
		if (dev) {
			addressLabel->setText(dev->address());
			QString status;
			if (dev->isConnected()) {
					status = i18n("Connected");
					connectButton->setText(i18n("Disconnect"));
			} else {
					status = i18n("Not connected");
					connectButton->setText(i18n("Connect"));
			}

			QString trustVal;
			if (dev->isTrusted()) {
					trustVal = i18n("Always trust");
					setTrustButton->setText(i18n("Remove Trust"));
			} else {
					trustVal = i18n("Not trusted");
					setTrustButton->setText(i18n("Set Trusted"));
			}

			trustLabel->setText(trustVal);
			statusLabel->setText(status);
			typeLabel->setText(dev->icon());
//		iconLabel->setPixmap(findIcon(remoteDev->majorClass(),remoteDev->minorClass()).pixmap(64,64));
			iconLabel->setPixmap(KIcon(dev->icon()).pixmap(64,64));
		}
	} else {
		connectButton->setEnabled(false);
		actionRemove->setEnabled(false);
		setTrustButton->setEnabled(false);
		removeButton->setEnabled(false);

		addressLabel->setText("");
		connectButton->setText(i18n("Connect"));
		statusLabel->setText("");
		typeLabel->setText("");
		trustLabel->setText("");
		iconLabel->setPixmap(0);
	}
}

void DeviceMan::slotConnectDevice(bool /*click*/)
{
	current = deviceListWidget->currentItem();
	if (!current)
		return;
	Solid::Control::BluetoothRemoteDevice* dev = 0;
	foreach(dev, *remoteDeviceMap) {
		if (current->text() == dev->name())
		break;
	}

	Solid::Control::BluetoothInputDevice* inputDev = iface->findBluetoothInputDeviceUBI(dev->ubi());

	if(dev->isConnected()){
		kDebug() << "The device is conencted, trying to disconnect";
		inputDev->disconnect();
		slotDevicePropertyChanged("Connected",false);
	}else{
		kDebug() << "Trying to disconnect";
		inputDev->connect();
		slotDevicePropertyChanged("Connected",true);
	}
}

void DeviceMan::slotRemoveDevice(bool /*click*/)
{
	kDebug() << "slotRemoveDevice" ;
	current = deviceListWidget->currentItem();
	if (!current)
		return;
	Solid::Control::BluetoothRemoteDevice* dev = 0;

	foreach(dev, *remoteDeviceMap) {
		if (current->text() == dev->name())
		break;
	}
	if (dev) {
		dev->disconnect();

		remoteDeviceMap->remove(dev->address());
		deviceListWidget->removeItemWidget(deviceListWidget->findItems(dev->name(),Qt::MatchExactly).value(0));
		deviceListWidget->takeItem(deviceListWidget->currentRow());
		current = 0;

		iface->removeDevice(dev->ubi());
		devList = iface->listDevices();
	}
}

void DeviceMan::slotDevicePropertyChanged(const QString& prop, const QVariant& value)
{
	kDebug() << "Property changed " << prop << value;
	if ( current && (current->text() == deviceListWidget->currentItem()->text()) ) {
		if (prop == "Connected") {
			bool conn = value.toBool();
			if (conn) {
				statusLabel->setText(i18n("Connected"));
				connectButton->setText(i18n("Disconnect"));
				statusbar->showMessage(i18n("Connected to %1",current->text()));
			} else {
				statusLabel->setText(i18n("Not connected"));
				connectButton->setText(i18n("Connect"));
				statusbar->showMessage(i18n("Disconnected from %1",current->text())); 
			}
		} else if (prop == "Trusted") {
			bool trust = value.toBool();
			if (trust) {
				trustLabel->setText(i18n("Always trust"));
				setTrustButton->setText(i18n("Remove Trust"));
				statusbar->showMessage(i18n("Permanent Trust added for %1",current->text())); 
			} else {
				trustLabel->setText(i18n("Not trusted"));
				setTrustButton->setText(i18n("Set Trusted"));
				statusbar->showMessage(i18n("Permanent Trust removed for %1",current->text()));
			}
		}
	}
}

KIcon DeviceMan::findIcon(QString majorClass, QString minorClass)
{
	if (majorClass == "peripheral") {
		if (minorClass == "pointing")
			return KIcon("input-mouse");
		if (minorClass == "keyboard")
			return KIcon("input-keyboard");

	} else if (majorClass == "phone")
		return KIcon("phone");

	return KIcon();
}

void DeviceMan::slotDeviceAdded(const QString& ubi)
{
	kDebug() << "Device Added";
		
	Solid::Control::BluetoothRemoteDevice* dev;
	foreach(dev, *remoteDeviceMap) {
		kDebug() << dev->name();
		if (dev->ubi() == ubi) {
			kDebug() << "Device " << ubi << "already exists.";
			return;
		} 
	} 

	Solid::Control::BluetoothRemoteDevice* remDevice = iface->findBluetoothRemoteDeviceUBI(ubi);

	QStringList uuids = remDevice->uuids();
	kDebug() << "UUIDS " << uuids;

	remoteDeviceMap->insert(remDevice->address(),remDevice);	

	devList = iface->listDevices();
	deviceListWidget->addItem(remDevice->name());

	connect(remDevice,SIGNAL(propertyChanged(const QString&, const QVariant&)),this,SLOT(slotDevicePropertyChanged(const QString&, const QVariant&)));
}

void DeviceMan::slotDeviceRemoved(const QString& ubi)
{
	kDebug() << "Device Removed";

	Solid::Control::BluetoothRemoteDevice* dev;
	foreach(dev, *remoteDeviceMap) {
		kDebug() << dev->name();
		if (dev->ubi() == ubi) {
			kDebug() << "Device " << dev->name() << " not removed ";
			break;
		} else 
			kDebug() << "Device " << ubi << " succesfully removed ";
	}
}

void DeviceMan::slotChangeTrust(bool /*clicked*/)
{
	
	current = deviceListWidget->currentItem();
	if (!current)
		return;
	Solid::Control::BluetoothRemoteDevice* dev = 0;
	foreach(dev, *remoteDeviceMap) {
		if (current->text() == dev->name())
			break;
	}
	if (dev)
		dev->setTrusted(!dev->isTrusted());
}

void DeviceMan::slotMenuActive()
{
	statusbar->clearMessage();
}

#include "devicemanager.moc"