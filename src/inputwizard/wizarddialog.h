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

#ifndef WIZARDDIALOG_H
#define WIZARDDIALOG_H

#include "wizardpage.h"

#include <QWizard>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>

#include <KConfig>
#include <kjob.h>
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothinterface.h>
#include <solid/control/bluetoothinputdevice.h>

class BTWizard :   public QWizard
{
Q_OBJECT

public:
	BTWizard(QApplication* parent = 0);
	~BTWizard();

private:
	const Solid::Control::BluetoothManager& btmanager;
	QApplication* m_parent;
	Solid::Control::BluetoothInterface* iface;
	Solid::Control::BluetoothRemoteDeviceList devList;
	Solid::Control::BluetoothRemoteDevice* remoteDev;
	WizardPage* introPage;
	QLabel* introLabel;
	QVBoxLayout* introLayout;

	WizardPage* discoveryPage;
	QLabel* discoveryLabel;
	QLabel* discoveryLabel1;
	QLabel* discoveryLabel2;
	QLabel* discoveryLabel3;
	QLabel* discoveryLabel4;
	QLabel* discoveryLabel5;
	QListWidget* discoveryWidget;
	QPushButton* discoveryButton;
	QVBoxLayout* discoveryLayout;
	QHBoxLayout* discoveryLayout1;

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
	WizardPage* createDiscoveryPage();
	WizardPage* createConnectionPage();
	WizardPage* createFinalPage();

	QString finalDev;
	QMap<QString, QMap< QString,QVariant > > macMap;
	
	QString service;

	QPixmap flashPixmap;
	QPixmap normalPixmap;
	QTimer* logoTimer;

//	QEventLoop m_loop;
//	int m_error;
//	QString m_errorString;

//	void createBonding();
//	void createInputDev();
	
	void addDeviceFound(const QString&, const QMap< QString,QVariant >&);
	void removeDeviceDisappeared(const QString&);
	void checkInputCapability(const QString&);
	void connectToDevice();

private slots:
	void slotCancel();
	void slotNext();	
	void slotSearch();
	void slotDeviceFound(const QString&, const QMap< QString,QVariant > &);
	void slotDeviceDisappeared(const QString&);
	void slotDeviceCreated(const QString&);
	void slotFinish();

//	void connectJob(KJob*);
//	void slotPercent(KJob* , unsigned long);
//	void slotInfoMessage(KJob*, const QString&);
//	void slotResult(KJob*);

	void slotDeviceConnected();
	void slotDeviceSelectionChanged();

	void slotDevicePropertyChanged(const QString&, const QVariant&);

	void setFlashingLogo();
	void setNormalLogo();
};
#endif
