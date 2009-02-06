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

#ifndef WIZARDDIALOG_H
#define WIZARDDIALOG_H

#include <KConfig>
#include <KConfigGroup>
#include <KIcon>
#include <QtGui>
#include <QWizard>

#include <kjob.h>

#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothinterface.h>
#include <solid/control/bluetoothinputdevice.h>

#include "wizardpage.h"


class BTWizard :   public QWizard
{
Q_OBJECT

public:
	BTWizard(QObject* parent = 0);
	~BTWizard();


private:
	const Solid::Control::BluetoothManager& btmanager;
        QObject* m_parent;
	Solid::Control::BluetoothInterface* iface;
        Solid::Control::BluetoothRemoteDeviceList devList;
        Solid::Control::BluetoothRemoteDevice* remoteDev;
	WizardPage* introPage;
	QLabel* introLabel;
	QVBoxLayout* introLayout;

	WizardPage* servicePage;
	QLabel* serviceLabel;
	QRadioButton* serviceRadioButton1;
	QRadioButton* serviceRadioButton2;
	QVBoxLayout* serviceLayout;

	WizardPage* devicePage;
	QLabel* deviceLabel;
	QLabel* deviceLabel1;
	QLabel* deviceLabel2;
	QLabel* deviceLabel3;
	QLabel* deviceLabel4;
	QLabel* deviceLabel5;
	QListWidget* deviceWidget;
	QPushButton* deviceButton;
	QComboBox* deviceComboBox;
	QVBoxLayout* deviceLayout;
	QHBoxLayout* deviceLayout1;
	QHBoxLayout* deviceLayout2;

	WizardPage* connPage;
	QLabel* connLabel;
	QLabel* connLabel1;
	QLabel* connLabel2;
	QLabel* connLabel3;
	QLabel* connLabel4;
	QLabel* connLabel5;
	QVBoxLayout* connLayout;
	
        WizardPage* finalPage;
        QLabel* finalLabel;
        QVBoxLayout* finalLayout;

	WizardPage* createIntroPage();
	WizardPage* createSelectServicePage();
	WizardPage* createSelectDevicePage();
	WizardPage* createConnectionPage();
	WizardPage* createFinalPage();

	QString finalDev;
	QMap<QString, QMap< QString,QVariant > > macMap;
	
	QListWidgetItem* item;

	QString service;

        QPixmap flashPixmap;
        QPixmap normalPixmap;
        QTimer* logoTimer;

//	QEventLoop m_loop;
//	int m_error;
//	QString m_errorString;

//	void createBonding();
//	void createInputDev();
	
	void filterList(const QString&, const QString&);
        void checkInputCapability(const QString&);
        void connectToDevice();

private slots:
	void slotCancel();
	void slotNext();	
	void slotSearch();
	void slotRemoteDeviceFound(const QString&, const QMap< QString,QVariant > &);
	void slotRemoteDeviceCreated(const QString&);
	void slotFinish();

//	void connectJob(KJob*);
//	void slotPercent(KJob* , unsigned long);
//	void slotInfoMessage(KJob*, const QString&);
//	void slotResult(KJob*);

	void slotDeviceConnected();
	void changeFilterView(const QString&);
	void slotDeviceSelectionChanged();

        void slotDevicePropertyChanged(const QString&, const QVariant&);

        void setFlashingLogo();
        void setNormalLogo();
	

};


#endif
