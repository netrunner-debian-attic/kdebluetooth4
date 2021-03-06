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

#include "devicesel.h"

#include <QTimer>

DeviceSel::DeviceSel(QObject* parent,QStringList filter) : Ui_deviceSelector(), m_parent(parent)
{
	item = 0;
	filterList = filter;

	setupUi(this);
	setWindowIcon(KIcon("kbluetooth"));
	connect(cancelButton,SIGNAL(clicked()),this,SLOT(slotCancel()));
	connect(sendButton,SIGNAL(clicked()),this,SLOT(slotSend()));
	connect(searchButton,SIGNAL(clicked()),this,SLOT(slotSearch()));
	connect(extraCheckBox,SIGNAL(toggled(bool)),this,SLOT(slotUnlockChecker(bool)));
	connect(deviceListWidget,SIGNAL(itemClicked(QListWidgetItem* )),this,SLOT(slotDeviceSelected(QListWidgetItem *)));

	Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();
	adapter = new Solid::Control::BluetoothInterface(man.defaultInterface());

	normalPixmap = KIcon("kbluetooth").pixmap(64,64);
	flashPixmap = KIcon("kbluetooth-flashing").pixmap(64,64);

	logoLabel->setPixmap(normalPixmap);
	logoLabel->setAlignment(Qt::AlignCenter);
	logoTimer = new QTimer(this);
	connect(logoTimer, SIGNAL(timeout()), this, SLOT(setFlashingLogo()));

	show();
	slotSearch();
}

DeviceSel::~DeviceSel()
{

	if (adapter) {
		adapter->stopDiscovery();
	}
	if (item)
		delete item;
	if (logoTimer) {
		logoTimer->stop();
		delete logoTimer;
	}
}


void DeviceSel::slotCancel()
{
	adapter->stopDiscovery();
	emit selectorCancelled();
	close();
}

void DeviceSel::slotRemoteDeviceFound(const QString& addr, const QMap<QString,QVariant>& props)
{
	qDebug() << "Remote Device found " << addr  << "Name: " << props["Name"];
	if (!macMap.contains(addr) && !props["Name"].toString().isEmpty()) {
		QString icon = props["Icon"].toString();
		QString name = props["Name"].toString();

		if (deviceListWidget->findItems(name + " - " + addr,Qt::MatchExactly ).count() > 0 ) 
			return;

		macMap[addr] = props;
		item = new QListWidgetItem(KIcon(icon),name + " - " + addr,deviceListWidget);
	}
}

void DeviceSel::configuredDevices(){
	Solid::Control::BluetoothRemoteDeviceList devList;
	devList = adapter->listDevices();
	qDebug() << "Device List Size: " << devList.size();
	if (devList.size() > 0) {
		Solid::Control::BluetoothRemoteDevice* dev;
		foreach(dev,devList) {
			qDebug() << dev->name();
			macMap[dev->address()] = dev->getProperties();
			item = new QListWidgetItem(KIcon(dev->icon()),dev->name() + " - " + dev->address(),deviceListWidget);
		}
	}
}

void DeviceSel::slotSearch()
{
	if (searchButton->text() == i18n("&Search")) {
		searchButton->setEnabled(true);
		searchButton->setText(i18n("&Stop Searching"));
		sendButton->setEnabled(false);
		deviceListWidget->clear();
		configuredDevices();
		adapter->startDiscovery();
		connect(adapter,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotRemoteDeviceFound(const QString&, const QMap< QString,QVariant > & )));
		logoTimer->start(400);
    } else {
		adapter->stopDiscovery();
		disconnect(adapter,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotRemoteDeviceFound(const QString&, const QMap< QString,QVariant > & )));
		searchButton->setEnabled(true);
		searchButton->setText(i18n("&Search"));
		logoTimer->stop();
		setNormalLogo();
		if (!deviceListWidget->selectedItems().count() > 0)
			sendButton->setEnabled(false);
	}
}

void DeviceSel::slotSend()
{
	adapter->stopDiscovery();
	QString mac = deviceListWidget->currentItem()->text().split(" - ").last();
	if (macMap.contains(mac))
		emit deviceSelected(mac);
	close();
}

void DeviceSel::setOkButtonText(const QString& txt)
{
	sendButton->setText(txt);
}

void DeviceSel::setInfoLabel(const QString& txt)
{
	infoLabel->setText(txt);
}

void DeviceSel::hideExtraCheckBox()
{
	extraCheckBox->hide();
}

void DeviceSel::slotUnlockChecker(bool check)
{
	emit unlockChanged(check);
}

void DeviceSel::slotDeviceSelected(QListWidgetItem* /* item */)
{
	sendButton->setEnabled(true);
}

void DeviceSel::setFlashingLogo()
{
	logoLabel->setPixmap(flashPixmap);
	QTimer::singleShot(200, this, SLOT(setNormalLogo()));
}

void DeviceSel::setNormalLogo()
{
	logoLabel->setPixmap(normalPixmap);
}