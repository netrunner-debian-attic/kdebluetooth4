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

#include "wizarddialog.h"

#include <QTimer>
#include <QPushButton>

#include <KDebug>
#include <klocalizedstring.h>
#include <KIcon>

#define HID "00001124-0000-1000-8000-00805F9B34FB"
#define PNP "00001200-0000-1000-8000-00805F9B34FB"
#define HEADSET "00001108-0000-1000-8000-00805F9B34FB"

#define AGENT_PATH "/kbluetooth4_wizard_agent"

enum
{
	BTAddressRole = Qt::UserRole + 42
};

BTWizard::BTWizard(QApplication* parent) : QWizard(), btmanager(Solid::Control::BluetoothManager::self()), m_parent(parent)
{
	iface = 0;
	introPage = 0;
	discoveryPage = 0;
	connPage = 0;
	finalPage = 0;

	iface = new Solid::Control::BluetoothInterface(btmanager.defaultInterface());

	addPage(createIntroPage());
	addPage(createDiscoveryPage());
	addPage(createConnectionPage());
	addPage(createFinalPage());
	setWindowIcon(KIcon("kbluetooth"));
	setWindowTitle(i18n("Bluetooth Device Wizard"));


	connect(button(QWizard::CancelButton),SIGNAL(clicked()),this,SLOT(slotCancel()));
	connect(button(QWizard::NextButton),SIGNAL(clicked()),this,SLOT(slotNext()));
	connect(button(QWizard::FinishButton),SIGNAL(clicked()),this,SLOT(slotFinish()));

	normalPixmap = KIcon("kbluetooth").pixmap(128,128);
	flashPixmap = KIcon("kbluetooth-flashing").pixmap(128,128);

	setPixmap(QWizard::WatermarkPixmap,normalPixmap);
	logoTimer = new QTimer(this);
	connect(logoTimer, SIGNAL(timeout()), this, SLOT(setFlashingLogo()));
	show();
}

BTWizard::~BTWizard()
{
	kDebug() << "Deleting Wizard";
	if (iface)
		iface->stopDiscovery();
}

WizardPage* BTWizard::createIntroPage()
{
	introPage = new WizardPage;

	introPage->setTitle(i18n("Setup Bluetooth Devices"));

	introLabel = new QLabel(i18n("This wizard will help to setup your Bluetooth devices."),introPage);
	introLabel->setWordWrap(true);

	introLayout = new QVBoxLayout(introPage);
	introLayout->addWidget(introLabel);
	introPage->setLayout(introLayout);

	introPage->setComplete(true);
	return introPage;
}

WizardPage* BTWizard::createDiscoveryPage()
{
	discoveryPage = new WizardPage;
	discoveryPage->setComplete(false);
	
	discoveryPage->setTitle(i18n("Select your Device"));

	discoveryLabel = new QLabel(i18n("Please select your Bluetooth device."),discoveryPage);
	discoveryLabel->setWordWrap(true);

	discoveryWidget = new QListWidget(discoveryPage);
	connect(discoveryWidget,SIGNAL(itemSelectionChanged()),this,SLOT(slotDeviceSelectionChanged()));

	discoveryLabel1 = new QLabel(i18n("<b>If your Device is not in the List:</b>"),discoveryPage);
	discoveryLabel2 = new QLabel(i18n("* Make sure bluetooth is enabled on the device"),discoveryPage);
	discoveryLabel3 = new QLabel(i18n("* Make sure the device is in discoverable mode"),discoveryPage);
	discoveryLabel4 = new QLabel(i18n("* Search again"),discoveryPage);
	discoveryButton = new QPushButton(i18n("&Search"),discoveryPage);

	discoveryLayout1 = new QHBoxLayout;
	discoveryLayout1->addWidget(discoveryButton);
	discoveryLayout1->addStretch();

	discoveryButton->setEnabled(false);
	connect(discoveryButton,SIGNAL(clicked()),this,SLOT(slotSearch()));

	discoveryLayout = new QVBoxLayout;
	discoveryLayout->addWidget(discoveryLabel);
	discoveryLayout->addWidget(discoveryWidget);
	discoveryLayout->addWidget(discoveryLabel1);
	discoveryLayout->addWidget(discoveryLabel2);
	discoveryLayout->addWidget(discoveryLabel3);
	discoveryLayout->addWidget(discoveryLabel4);

	discoveryLayout->addLayout(discoveryLayout1);

	discoveryPage->setLayout(discoveryLayout);

	return discoveryPage;
}

WizardPage* BTWizard::createConnectionPage()
{
	connPage = new WizardPage;
	connPage->setComplete(false);
	connPage->setTitle(i18n("Summary"));
	connLabel = new QLabel(i18n("Connection will be established to this Bluetooth device"),connPage);
	connLabel->setWordWrap(true);


	connLabel1 = new QLabel(connPage);
	connLabel2 = new QLabel(connPage);
	connLabel3 = new QLabel(connPage);
	connLabel4 = new QLabel(connPage);
	connLabel5 = new QLabel(connPage);
		
	connLayout = new QVBoxLayout(connPage);
	connLayout->addWidget(connLabel);
	connLayout->addWidget(connLabel1);
	connLayout->addWidget(connLabel2);
	connLayout->addWidget(connLabel3);
	connLayout->addWidget(connLabel4);
	connLayout->addWidget(connLabel5);

	connPage->setLayout(connLayout);

	//connect(connPage,SIGNAL(completeChanged()),this,SLOT(slotValidatePage()));

	return connPage;
}

WizardPage* BTWizard::createFinalPage()
{
	finalPage = new WizardPage;
	finalPage->setComplete(false);
	finalPage->setTitle(i18n("Finished"));
	finalLabel = new QLabel(i18n("Please wait, until the connection to your Bluetooth Device is done ..."),connPage);
	finalLabel->setWordWrap(true);
	finalLayout = new QVBoxLayout(finalPage);
	finalLayout->addWidget(finalLabel);

	finalPage->setLayout(finalLayout);

	return finalPage;
}


void BTWizard::slotDeviceFound(const QString& addr, const QMap< QString,QVariant > & props)
{
	kDebug() << "Configured Devices from slotDeviceFound: " << devList.size();
	foreach(Solid::Control::BluetoothRemoteDevice* dev, devList) {
		if (dev->address() == addr)
			return;
	}

//      kDebug() << props;
	QString address = addr;
	QString alias = props["Alias"].toString();
	QList<QListWidgetItem *> l;

	l = discoveryWidget->findItems(alias, Qt::MatchExactly);

	foreach (QListWidgetItem *i, l) {
		if (i->data(BTAddressRole).toString() == address)
			return;
	}

	addDeviceFound(addr, props);
	macMap.insert(address, props);
}

void BTWizard::slotDeviceDisappeared(const QString& addr)
{
	kDebug() << "Configured Devices from slotDeviceDisappeared: " << devList.size();
	foreach(Solid::Control::BluetoothRemoteDevice* dev, devList) {
		if (dev->address() == addr)
			return;
	}

	QString address = addr;

	removeDeviceDisappeared(addr);
	macMap.remove(address);
}

void BTWizard::slotDeviceSelectionChanged()
{
	kDebug() << discoveryWidget->currentItem();
	discoveryPage->setComplete( discoveryWidget->currentItem() ? true : false );
}

void BTWizard::slotSearch()
{
	kDebug() << "Text " << discoveryButton->text();
	if (discoveryButton->text() == i18n("&Search")) {
		kDebug() << "searching..." ;
		devList = iface->listDevices();
		kDebug() << "Configured Devices: " << devList.size();
		discoveryPage->setComplete(false);
		discoveryButton->setEnabled(true);
		discoveryButton->setText(i18n("&Stop Search"));
		iface->startDiscovery();
		discoveryWidget->clear();
		connect(iface,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotDeviceFound(const QString&, const QMap< QString,QVariant > & )));
		connect(iface,SIGNAL(deviceDisappeared( const QString& )),this,SLOT(slotDeviceDisappeared(const QString&)));
		logoTimer->start(400);
	} else {
		kDebug() << "Stop searching" ;
		discoveryPage->setComplete(false);
		discoveryButton->setEnabled(true);
		discoveryButton->setText(i18n("&Search"));
		iface->stopDiscovery();
		disconnect(iface,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotDeviceFound(const QString&, const QMap< QString,QVariant > & )));
		disconnect(iface,SIGNAL(deviceDisappeared( const QString&)),this,SLOT(slotDeviceDisappeared(const QString&)));
		discoveryPage->setComplete( discoveryWidget->currentItem() ? true : false );
		logoTimer->stop();
		setNormalLogo();
	}
}

void BTWizard::slotNext()
{
	kDebug() << "Page ID: " << currentId();
	switch (currentId()) {
		case 1 : {
			logoTimer->stop();
			setNormalLogo();
			kDebug() << "page 2 reached!";
			slotSearch();
			break;
		}
		case 2 : {
			iface->stopDiscovery();
			logoTimer->stop();
			setNormalLogo();
			disconnect(iface,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotDeviceFound(const QString&, const QMap< QString,QVariant > & )));
			disconnect(iface,SIGNAL(deviceDisappeared( const QString& )),this,SLOT(slotDeviceDisappeared(const QString&)));
			if(discoveryWidget->currentItem()) {
				QString addr = discoveryWidget->currentItem()->data(BTAddressRole).toString();
				finalDev = addr;
				connLabel->setText(i18n("Checking your Bluetooth Device for input capabilities..."));
				connLabel1->setText("");
				connLabel2->setText("");
				connLabel3->setText("");
				connLabel4->setText("");
				connPage->setComplete(false);
				checkInputCapability(finalDev);
			}
			break;
		}
		case 3:
			connectToDevice();
	}
}

void BTWizard::checkInputCapability(const QString& addr)
{
	iface->createBluetoothRemoteDevice(addr);
	connect(iface,SIGNAL(deviceCreated(const QString &)),this,SLOT(slotDeviceCreated(const QString &)));
}

void BTWizard::connectToDevice()
{
	kDebug() << "trying to finish...";
	if (macMap[finalDev].contains("UBI")) {
		QString icon = macMap[finalDev]["Icon"].toString();
		kDebug() << "trying to connect... " << finalDev << macMap[finalDev]["Name"].toString();
		disconnect(iface,SIGNAL(deviceCreated(const QString &)),this,SLOT(slotDeviceCreated(const QString &)));
		Solid::Control::BluetoothInputDevice* inputDev = iface->findBluetoothInputDeviceUBI(macMap[finalDev]["UBI"].toString());

		inputDev->connect();
		slotDevicePropertyChanged("Connected",true);
	}
}

void BTWizard::slotDevicePropertyChanged(const QString& prop, const QVariant& value)
{
	kDebug() << "Property Changed " << prop << " " << value;
	//disconnect(remoteDev,SIGNAL(propertyChanged(const QString&, const QVariant&)),this,SLOT(slotDevicePropertyChanged(const QString&, const QVariant&)));
	if((prop == "Connected" || prop == "Paired") && value.toBool() == true) {
		finalPage->setComplete(true);
		finalLabel->setText(i18n("Your Device is setup properly and should work now."));
	}
}

void BTWizard::slotFinish()
{
	close();
}


void BTWizard::slotDeviceCreated(const QString& ubi)
{
	kDebug() << "device created " << ubi;

	disconnect(iface,SIGNAL(deviceCreated(const QString &)),this,SLOT(slotDeviceCreated(const QString &)));

	Solid::Control::BluetoothRemoteDevice* dev = iface->findBluetoothRemoteDeviceUBI(ubi);

	//CreatePairedDevice Mode /// 
	/*
	if (macMap[finalDev].contains("UBI")) {
			Solid::Control::BluetoothInputDevice* inputDev = iface->findBluetoothInputDeviceUBI(ubi);
			inputDev->connect();
			connect(dev,SIGNAL(propertyChanged(const QString&, const QVariant&)),this,SLOT(slotDevicePropertyChanged(const QString&, const QVariant&)));
	} else {  */

	QStringList uuids = dev->uuids();
	if (uuids.contains(HID,Qt::CaseInsensitive)) {

		connLabel->setText(i18n("Connection will be established to this Bluetooth device"));
		connLabel1->setText(i18n("<b>Name: </b>%1",(dev->name())));
		connLabel2->setText(i18n("<b>Address: </b>%1",(dev->address())));
		connLabel3->setText(i18n("<b>Device class: </b>%1",(dev->icon())));
		connLabel4->setPixmap(KIcon(dev->icon()).pixmap(32,32));
		connPage->setComplete(true);
		//iface->removeDevice(ubi);
		macMap[dev->address()]["UBI"] = ubi;

	} else {
		iface->removeDevice(ubi);
		connLabel->setText(i18n("Sorry your Bluetooth Device does not support input Service."));
		connLabel1->setText("");
		connLabel2->setText("");
		connLabel3->setText("");
		connLabel4->setText("");
		connPage->setComplete(false);
	}
//      }
}

void BTWizard::slotDeviceConnected()
{
	kDebug() << "device connected ";
}

void BTWizard::slotCancel()
{
	iface->stopDiscovery();
	close();
}


void BTWizard::addDeviceFound(const QString& addr, const QMap< QString,QVariant > & props)
{
	QString icon = props["Icon"].toString();
	QString alias = props["Alias"].toString();
	QString address = addr;

	kDebug() << "addDeviceFound " << alias << " " << address;

	if (macMap.contains(addr)) {
		QString old = macMap[addr]["Alias"].toString();
		QList<QListWidgetItem *> l;
		l = discoveryWidget->findItems(old, Qt::MatchExactly);
		foreach (QListWidgetItem *i, l) {
			if (i->data(BTAddressRole).toString() == addr) {
				i->setText(alias);
				return;
			}
		}
	}

	QListWidgetItem *item = new QListWidgetItem(KIcon(icon), alias, discoveryWidget);
	item->setData(BTAddressRole, addr);
	discoveryWidget->addItem(item);
}

void BTWizard::removeDeviceDisappeared(const QString& addr)
{
	QString alias = macMap[addr]["Alias"].toString();
	QString address = addr;
	QList<QListWidgetItem *> l;

	kDebug() << "removeDeviceDisappeared " << alias << " " << address;

	l = discoveryWidget->findItems(alias, Qt::MatchExactly);

	if (l.isEmpty())
		return;

	foreach (QListWidgetItem *i, l) {
		if (i->data(BTAddressRole).toString() == address)
			delete i;
	}
}

void BTWizard::setFlashingLogo()
{
	setPixmap(QWizard::WatermarkPixmap, flashPixmap);
	QTimer::singleShot(200, this, SLOT(setNormalLogo()));
}

void BTWizard::setNormalLogo()
{
	setPixmap(QWizard::WatermarkPixmap, normalPixmap);
}


