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


#include <KDebug>
#include "adapterconfig.h"

AdapterConfig::AdapterConfig(QObject* parent) : Ui_AdapterUi(),btmanager(Solid::Control::BluetoothManager::self())
{
	m_parent = parent;
	setupUi(this);
	setWindowIcon(KIcon("kbluetooth4"));
	adapterTabWidget->clear();
	closePushButton->setIcon(KIcon("dialog-ok"));
	connect(closePushButton,SIGNAL(clicked()),this, SLOT(slotQuit()));	

	connect(&btmanager,SIGNAL(interfaceAdded(const QString&)),this,SLOT(adapterAdded(const QString&)));
	connect(&btmanager,SIGNAL(interfaceRemoved(const QString&)),this,SLOT(adapterRemoved(const QString&)));
	
	adapterMap = new QMap<QString,AdapterWidget*>();

	initialize();
	
}

AdapterConfig::~AdapterConfig()
{
	qDeleteAll (adapterMap->begin(),adapterMap->end());
	delete adapterMap;
}

void AdapterConfig::initialize()
{
	searchAdapters();
	show();
}

void AdapterConfig::searchAdapters()
{
	Solid::Control::BluetoothInterfaceList adapters = btmanager.bluetoothInterfaces();	
	foreach (Solid::Control::BluetoothInterface dev,adapters) {

		if(adapterMap->find(dev.ubi()) == adapterMap->end()) {

			kDebug() << "New Adapter: " << dev.ubi();
			tab = new AdapterWidget(dev.ubi(),this);
			connect(tab,SIGNAL(nameFieldChanged(AdapterWidget*, QString)),this,SLOT(setTabName(AdapterWidget*,QString)));
			setAdapterWidgetProperties(tab,dev);
			adapterMap->insert(dev.ubi(),tab);
			adapterTabWidget->addTab(tab,dev.name());
		} else {
			kDebug() << "Adapter: " << dev.ubi();
			AdapterWidget* tab1= *(adapterMap->find(dev.ubi()));
			setAdapterWidgetProperties(tab1,dev);
		}
	}
}

void AdapterConfig::setAdapterWidgetProperties(AdapterWidget* tab,Solid::Control::BluetoothInterface dev)
{
		tab->setMac(dev.address());
//bluez4//		tab->setVersion(dev.version());
//bluez4//		tab->setDetails(dev.revision(),dev.manufacturer());

		tab->setName(dev.name());
		tab->setDiscoverable(dev.isDiscoverable());
		tab->setDiscoverableTimeout(dev.discoverableTimeout());
//bluez4//		tab->setMinorClass(dev.minorClass());
}

void AdapterConfig::setTabName(AdapterWidget* tab, QString name)
{
	int index = adapterTabWidget->indexOf(tab);
	adapterTabWidget->setTabText(index,name);
}

void AdapterConfig::adapterAdded(const QString& ubi)
{
	kDebug() << "Adapter added: " << ubi;
	Solid::Control::BluetoothInterface dev(ubi);
	tab = new AdapterWidget(ubi,this);
	connect(tab,SIGNAL(nameFieldChanged(AdapterWidget*, QString)),this,SLOT(setTabName(AdapterWidget*,QString)));
	setAdapterWidgetProperties(tab,dev);
	adapterMap->insert(ubi,tab);
	adapterTabWidget->addTab(tab,dev.name());
}

void AdapterConfig::adapterRemoved(const QString& ubi)
{
	kDebug() << "Adapter removed: " << ubi;
	AdapterWidget* tab = adapterMap->value(ubi);
	int index = adapterTabWidget->indexOf(tab);
	adapterTabWidget->removeTab(index);
	adapterMap->take(ubi);
	delete tab;
}

void AdapterConfig::slotQuit()
{
	close();
}

#include "moc_adapterconfig.cpp"
