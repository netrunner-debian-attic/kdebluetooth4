/*
 *
 *  KBluetooth4 - KDE4 Bluetooth Framework
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

#include "wizarddialog.h"
#include <KDebug>
#include <klocalizedstring.h>
#include <KMessageBox>


#define HID "00001124-0000-1000-8000-00805F9B34FB"
#define PNP "00001200-0000-1000-8000-00805F9B34FB"
#define HEADSET "00001108-0000-1000-8000-00805F9B34FB"

#define AGENT_PATH "/kbluetooth4_wizard_agent"


BTWizard::BTWizard(QObject* parent) : QWizard(), btmanager(Solid::Control::BluetoothManager::self()), m_parent(parent)
{
	iface = new Solid::Control::BluetoothInterface(btmanager.defaultInterface());

	addPage(createIntroPage());
	addPage(createSelectServicePage());
	addPage(createSelectDevicePage());
	addPage(createConnectionPage());
        addPage(createFinalPage());
	setWindowIcon(KIcon("kbluetooth4"));
	setWindowTitle("Bluetooth Device Wizard");

	
	connect(button(QWizard::CancelButton),SIGNAL(clicked()),this,SLOT(slotCancel()));
	connect(button(QWizard::NextButton),SIGNAL(clicked()),this,SLOT(slotNext()));
	connect(button(QWizard::FinishButton),SIGNAL(clicked()),this,SLOT(slotFinish()));

        normalPixmap = KIcon("kbluetooth4").pixmap(128,128);
        flashPixmap = KIcon("kbluetooth4-flashing").pixmap(128,128);

	setPixmap(QWizard::WatermarkPixmap,normalPixmap);
        logoTimer = new QTimer(this);
        connect(logoTimer, SIGNAL(timeout()), this, SLOT(setFlashingLogo()));
	show();
}

BTWizard::~BTWizard()
{
	kDebug() << "Deleting Wizard";

        iface->stopDiscovery();
	if (introPage)
		delete introPage;
	if (servicePage)
		delete servicePage;
	if (devicePage)
		delete devicePage;
	if (connPage)
		delete connPage;
        if (finalPage)
                delete finalPage;
	
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

WizardPage* BTWizard::createSelectServicePage()
{
	servicePage = new WizardPage;
	
	servicePage->setTitle(i18n("Select Device type"));
	
	serviceLabel = new QLabel(i18n("Please select the type of your Bluetooth device."),servicePage);
	serviceLabel->setWordWrap(true);
	
	serviceRadioButton1 = new QRadioButton(i18n("Input Device"),servicePage);
	serviceRadioButton2 = new QRadioButton(i18n("Audio Device"),servicePage);
	serviceLayout = new QVBoxLayout(servicePage);
	serviceLayout->addWidget(serviceLabel);
	serviceLayout->addWidget(serviceRadioButton1);
	serviceLayout->addWidget(serviceRadioButton2);
	servicePage->setLayout(serviceLayout);
	
	serviceRadioButton2->setEnabled(false);
	serviceRadioButton1->setChecked(true);
	
	return servicePage;
	
}

WizardPage* BTWizard::createSelectDevicePage()
{
	devicePage = new WizardPage;
	devicePage->setComplete(false);
	
	devicePage->setTitle(i18n("Select your Device"));

        deviceLabel = new QLabel(i18n("Please select your Bluetooth device."),devicePage);
        deviceLabel->setWordWrap(true);

	deviceWidget = new QListWidget(devicePage);
	connect(deviceWidget,SIGNAL(itemSelectionChanged()),this,SLOT(slotDeviceSelectionChanged()));

	deviceLabel1 = new QLabel(i18n("<b>If your Device is not in the List:</b>"),devicePage);
	deviceLabel2 = new QLabel(i18n("* Make sure bluetooth is enabled on the device"),devicePage);
	deviceLabel3 = new QLabel(i18n("* Make sure the device is in discoverable mode"),devicePage);
	deviceLabel4 = new QLabel(i18n("* Search again"),devicePage);
	deviceButton = new QPushButton("&Search",devicePage);
	deviceComboBox = new QComboBox(devicePage);
	
	deviceComboBox->addItem(i18n("All"));
	deviceComboBox->addItem(i18n("Keyboard/Mouse"));
	deviceComboBox->addItem(i18n("Phones"));
	deviceLabel5 = new QLabel(i18n("Filter:"),devicePage);

	deviceLayout2 = new QHBoxLayout;
	deviceLayout2->addWidget(deviceButton);
	deviceLayout2->addStretch();
	deviceLayout2->addWidget(deviceLabel5);
	deviceLayout2->addWidget(deviceComboBox);
	connect(deviceComboBox,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(changeFilterView(const QString&)));

	deviceButton->setEnabled(false);
	connect(deviceButton,SIGNAL(clicked()),this,SLOT(slotSearch()));

        deviceLayout = new QVBoxLayout;
        deviceLayout->addWidget(deviceLabel);
        deviceLayout->addWidget(deviceWidget);
        deviceLayout->addWidget(deviceLabel1);
        deviceLayout->addWidget(deviceLabel2);
        deviceLayout->addWidget(deviceLabel3);
        deviceLayout->addWidget(deviceLabel4);

        deviceLayout->addLayout(deviceLayout2);

        devicePage->setLayout(deviceLayout);

        return devicePage;

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
    finalLabel = new QLabel("Please wait, until the connection to your Bluetooth Device is done ...",connPage);
    finalLabel->setWordWrap(true);
    finalLayout = new QVBoxLayout(finalPage);
    finalLayout->addWidget(finalLabel);

    finalPage->setLayout(finalLayout);

    return finalPage;

}


void BTWizard::slotRemoteDeviceFound(const QString& addr, const QMap< QString,QVariant > & props)
{
        
        kDebug() << "Configured Devices from slotRemoteDeviceFound: " << devList.size();
        foreach(Solid::Control::BluetoothRemoteDevice* dev, devList) {
            if (dev->address() == addr)
                return;
        }

	item = 0;
//      kDebug() << props;
        QString address = addr;
        QString name = props["Name"].toString();

        if (name.isEmpty())
            return;

	if (macMap.contains(addr) && (deviceWidget->findItems(name + " - " + address,Qt::MatchExactly ).count() > 0 )) 
		return;

        macMap.insert(address,props);
        QString filter = deviceComboBox->currentText();
        filterList(addr,filter);
}


void BTWizard::slotDeviceSelectionChanged()
{
	kDebug() << deviceWidget->currentItem();
        devicePage->setComplete( deviceWidget->currentItem() ? true : false );
}

void BTWizard::slotSearch()
{
        kDebug() << "Text " << deviceButton->text();
        if (deviceButton->text() == "&Search") {
            kDebug() << "searching..." ;
	    devList = iface->listDevices();
            kDebug() << "Configured Devices: " << devList.size();
            devicePage->setComplete(false);
            deviceButton->setEnabled(true);
            deviceButton->setText("&Stop Search");
            iface->startDiscovery();
            deviceWidget->clear();
            connect(iface,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotRemoteDeviceFound(const QString&, const QMap< QString,QVariant > & )));
            logoTimer->start(400);
        } else {
            kDebug() << "Stop searching" ;
            devicePage->setComplete(false);
            deviceButton->setEnabled(true);
            deviceButton->setText("&Search");
            iface->stopDiscovery();
            disconnect(iface,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotRemoteDeviceFound(const QString&, const QMap< QString,QVariant > & )));
	    devicePage->setComplete( deviceWidget->currentItem() ? true : false );
            logoTimer->stop();
            setNormalLogo();

        }
}

void BTWizard::slotNext()
{
	kDebug() << "Page ID: " << currentId();
	switch (currentId()) {

		case 2 : {
                        logoTimer->stop();
                        setNormalLogo();
			if (serviceRadioButton1->isChecked()) {
				service = "input";
                                servicePage->setComplete(true);
                        }
			if (serviceRadioButton2->isChecked()) {
				service = "audio";
                                servicePage->setComplete(true);
                        }
			kDebug() << "page 2 reached!";
			slotSearch();
			break;
                }
		case 3 : {
                        iface->stopDiscovery();
                        logoTimer->stop();
                        setNormalLogo();
                        disconnect(iface,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotRemoteDeviceFound(const QString&, const QMap< QString,QVariant > & )));
			if(deviceWidget->currentItem()) {
				QString addr = deviceWidget->currentItem()->text().split(" - ").at(1);
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
                case 4:
                        connectToDevice();
	}
}

void BTWizard::checkInputCapability(const QString& addr)
{
    iface->createBluetoothRemoteDevice(addr);
    connect(iface,SIGNAL(deviceCreated(const QString &)),this,SLOT(slotRemoteDeviceCreated(const QString &)));
}

void BTWizard::connectToDevice()
{
	kDebug() << "trying to finish...";
        if (macMap[finalDev].contains("UBI")) {
            QString icon = macMap[finalDev]["Icon"].toString();
            kDebug() << "trying to connect... " << finalDev << macMap[finalDev]["Name"].toString();
            disconnect(iface,SIGNAL(deviceCreated(const QString &)),this,SLOT(slotRemoteDeviceCreated(const QString &)));
            Solid::Control::BluetoothInputDevice* inputDev = iface->findBluetoothInputDeviceUBI(macMap[finalDev]["UBI"].toString());
            remoteDev = iface->findBluetoothRemoteDeviceUBI(macMap[finalDev]["UBI"].toString());

            //// CreatePairedDevice ///// 
            /*
            if (icon == "input-keyboard" || icon == "phone") {
                iface->createPairedDevice(finalDev,AGENT_PATH,"DisplayYesNo");
                connect(iface,SIGNAL(deviceCreated(const QString &)),this,SLOT(slotRemoteDeviceCreated(const QString &)));
            } else {   */
                inputDev->connect();
                slotDevicePropertyChanged("Connected",true);
//          }



            
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


void BTWizard::slotRemoteDeviceCreated(const QString& ubi)
{
	kDebug() << "device created " << ubi;

        disconnect(iface,SIGNAL(deviceCreated(const QString &)),this,SLOT(slotRemoteDeviceCreated(const QString &)));

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
                //              iface->removeDevice(ubi);
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

void BTWizard::changeFilterView(const QString& filter)
{
	deviceWidget->clear();
		
	foreach(QString addr, macMap.keys()) {
		filterList(addr, filter);
	}
}

void BTWizard::filterList(const QString& addr, const QString& filter)
{
	QString icon = macMap[addr]["Icon"].toString();
	QString name = macMap[addr]["Name"].toString();
	QString address = addr;
        kDebug() << "Filter DeviceList " << name << " " << icon;
        if (icon == "input-keyboard" && (filter == "Keyboard/Mouse" || filter == "All")) {
            item = new QListWidgetItem(KIcon(icon),name + " - " + address,deviceWidget);
            deviceWidget->addItem(item);
        } else if ( icon == "input-mouse" && (filter == "Keyboard/Mouse" || filter == "All")) {
            item = new QListWidgetItem(KIcon(icon),name + " - " + address,deviceWidget);
            deviceWidget->addItem(item);
        } else if ( icon == "phone" && (filter == "Phones" || filter == "All")) {
            item = new QListWidgetItem(KIcon(icon),name + " - " + address,deviceWidget);
            deviceWidget->addItem(item);
        } else if ( icon == "computer" && filter == "All") {
            item = new QListWidgetItem(KIcon(icon),name + " - " + address,deviceWidget);
            deviceWidget->addItem(item);
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


