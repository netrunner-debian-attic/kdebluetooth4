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


#include "devicemanager.h"
#include <KDebug>
#include <KApplication>
#include <KProcess>
#include <KMessageBox>
#include <KLocale>
#include <QList>

#include <kjob.h>

#define HID "00001124-0000-1000-8000-00805F9B34FB"
#define FTP "00001106-0000-1000-8000-00805F9B34FB"

DeviceMan::DeviceMan(QObject* parent) : Ui_DeviceManager(), m_parent(parent)
{
//	 setAttribute(Qt::WA_DeleteOnClose);

	 Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();

	 iface = new Solid::Control::BluetoothInterface(man.defaultInterface());

	 connect(iface,SIGNAL(deviceCreated(const QString&)),this,SLOT(slotDeviceAdded(const QString&)));
	 connect(iface,SIGNAL(deviceRemoved(const QString&)),this,SLOT(slotDeviceRemoved(const QString&)));

	 kDebug() << "Device Manager - Welcome";
//	 setParent(kapp);
	 setupUi(this);
	 setWindowIcon(KIcon("kbluetooth4"));

//	 connect(closeButton,SIGNAL(clicked(bool)),this,SLOT(slotQuitApp(bool)));
	 connect(newDevButton,SIGNAL(clicked(bool)),this,SLOT(slotStartWizard(bool)));
	 connect(removeButton,SIGNAL(clicked(bool)),this,SLOT(slotRemoveDevice(bool)));

	 connect(actionConnect,SIGNAL(triggered(bool)),this,SLOT(slotConnectDevice(bool)));
	 connect(actionRemove_Trust,SIGNAL(triggered(bool)),this,SLOT(slotChangeTrust(bool)));
	 connect(actionRemove,SIGNAL(triggered(bool)),this,SLOT(slotRemoveDevice(bool)));
	 connect(actionNew,SIGNAL(triggered(bool)),this,SLOT(slotStartWizard(bool)));

	 connect(menuEdit,SIGNAL(aboutToShow()),this,SLOT(slotMenuActive()));
	 
	 connect(deviceListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotShowDetails()));

	 inputDeviceMap = new QMap<QString,Solid::Control::BluetoothInputDevice*>;
	 remoteDeviceMap = new QMap<QString,Solid::Control::BluetoothRemoteDevice*>;
	 show();
	 actionConnect->setEnabled(false);
	 actionRemove->setEnabled(false);
	 actionRemove_Trust->setEnabled(false);
	 removeButton->setEnabled(false);
	 
	 actionQuit->setIcon(KIcon("window-close"));
	 actionConnect->setIcon(KIcon("applications-internet"));
	 actionRemove->setIcon(KIcon("user-trash"));
	 actionNew->setIcon(KIcon("list-add"));
	 actionRemove_Trust->setIcon(KIcon("dialog-password"));
	 
	 current = 0;
	 
	 getConfiguredDevices();
}

DeviceMan::~DeviceMan()
{
	if ( inputDeviceMap->size() > 0 ) {
		qDeleteAll (inputDeviceMap->begin(),inputDeviceMap->end());
		inputDeviceMap->clear();
	}
	if (inputDeviceMap)
  //      	delete inputDeviceMap;
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
	process.setProgram("kbluetooth4-inputwizard");
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
                        if (uuids.contains(HID,Qt::CaseInsensitive)) {
			    deviceListWidget->addItem(dev->name());
			    Solid::Control::BluetoothInputDevice* device = iface->findBluetoothInputDeviceUBI(dev->ubi());
			    inputDeviceMap->insert(dev->address(),device);	
			    Solid::Control::BluetoothRemoteDevice* rem = iface->findBluetoothRemoteDeviceUBI(dev->ubi());
			    remoteDeviceMap->insert(dev->address(),rem);	
                        }
		}

		Solid::Control::BluetoothRemoteDevice* remDev;
		foreach(remDev,remoteDeviceMap->values()) {
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
	if(deviceListWidget->selectedItems().size() == 0) {
		current = 0;
	}
	statusbar->clearMessage();
	if (current) {
		actionConnect->setEnabled(true);
		actionRemove->setEnabled(true);
		actionRemove_Trust->setEnabled(true);
		removeButton->setEnabled(true);

		Solid::Control::BluetoothRemoteDevice* dev = 0;
		foreach(dev,remoteDeviceMap->values()) {
			if (current->text() == dev->name())
				break;
		}

//		Solid::Control::BluetoothRemoteDevice* remoteDev = remoteDeviceMap->value(dev.address());
		if (dev) {
                    addressLabel->setText(dev->address());
                    QString status;
                    if (dev->isConnected()) {
                            status = tr("Connected");
                            actionConnect->setText("Disconnect");
                    } else {
                            status = tr("Not connected");
                            actionConnect->setText("Connect");
                    }

                    QString trustVal;
                    if (dev->isTrusted()) {
                            trustVal = tr("Always trust");
                            actionRemove_Trust->setText(tr("Remove Trust"));
                    } else {
                            trustVal = tr("Not trusted");
                            actionRemove_Trust->setText(tr("Set Trusted"));
                    }

                    trustLabel->setText(trustVal);
                    statusLabel->setText(status);
                    typeLabel->setText(dev->icon());
    //		iconLabel->setPixmap(findIcon(remoteDev->majorClass(),remoteDev->minorClass()).pixmap(64,64));
                    iconLabel->setPixmap(KIcon(dev->icon()).pixmap(64,64));
                }

	} else {
		actionConnect->setEnabled(false);
		actionRemove->setEnabled(false);
		actionRemove_Trust->setEnabled(false);
		removeButton->setEnabled(false);

		addressLabel->setText("");
		actionConnect->setText(tr("Connect"));
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
        foreach(dev,remoteDeviceMap->values()) {
                if (current->text() == dev->name())
                        break;
        }
        if (dev) {
            Solid::Control::BluetoothInputDevice* inputDev = inputDeviceMap->value(dev->address());
            if (dev->isConnected()) {
                    inputDev->disconnect();
                    kDebug() << "Device disconnected";
            } else {
                    inputDev->connect();
                    kDebug() << "Device connected";
            }
        }
}

void DeviceMan::slotRemoveDevice(bool /*click*/)
{
	kDebug() << "slotRemoveDevice" ;
	current = deviceListWidget->currentItem();
	if (!current)
		return;
	Solid::Control::BluetoothRemoteDevice* dev = 0;

        foreach(dev,remoteDeviceMap->values()) {
                if (current->text() == dev->name())
                        break;
        }
        if (dev) {
            dev->disconnect();
    /*
            if(remoteDeviceMap->value(dev.address())->hasBonding()) {
                    remoteDeviceMap->value(dev.address())->removeBonding();
            }
    */

            inputDeviceMap->remove(dev->address());
            remoteDeviceMap->remove(dev->address());
            deviceListWidget->removeItemWidget(deviceListWidget->findItems(dev->name(),Qt::MatchExactly).at(0));
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
		statusLabel->setText(tr("Connected"));
		actionConnect->setText(tr("Disconnect"));
		statusbar->showMessage(tr("Connected to ") + current->text());
            } else {
		statusLabel->setText(tr("Not connected"));
		actionConnect->setText(tr("Connect"));
		statusbar->showMessage(tr("Disconnected from ") + current->text()); 
            }

        } else if (prop == "Trusted") {
            bool trust = value.toBool();
            if (trust) {
		trustLabel->setText(tr("Always trust"));
		actionRemove_Trust->setText(tr("Remove Trust"));
		statusbar->showMessage(tr("Permanent Trust added for ") + current->text()); 
            } else {
		trustLabel->setText(tr("Not trusted"));
		actionRemove_Trust->setText(tr("Set Trusted"));
		statusbar->showMessage(tr("Permanent Trust removed for ") + current->text());
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
	foreach(dev,remoteDeviceMap->values()) {
		kDebug() << dev->name();
		if (dev->ubi() == ubi) {
			kDebug() << "Device " << ubi << "already exists.";
			return;
		} 
	} 

	Solid::Control::BluetoothRemoteDevice* remDevice = iface->findBluetoothRemoteDeviceUBI(ubi);
	Solid::Control::BluetoothInputDevice* inputDevice = iface->findBluetoothInputDeviceUBI(ubi);

        QStringList uuids = remDevice->uuids();
        kDebug() << "UUIDS " << uuids;
        if (uuids.contains(HID,Qt::CaseInsensitive) || inputDevice != 0) {

            Solid::Control::BluetoothInputDevice* device = iface->findBluetoothInputDeviceUBI(ubi);

            inputDeviceMap->insert(remDevice->address(),device);	
            remoteDeviceMap->insert(remDevice->address(),remDevice);	

            devList = iface->listDevices();
            deviceListWidget->addItem(remDevice->name());

            connect(remDevice,SIGNAL(propertyChanged(const QString&, const QVariant&)),this,SLOT(slotDevicePropertyChanged(const QString&, const QVariant&)));
        }
}

void DeviceMan::slotDeviceRemoved(const QString& ubi)
{
	kDebug() << "Device Removed";

	Solid::Control::BluetoothRemoteDevice* dev;
	foreach(dev,remoteDeviceMap->values()) {
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
        foreach(dev,remoteDeviceMap->values()) {
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
