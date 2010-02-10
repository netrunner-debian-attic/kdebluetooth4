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

#include "adapterconfig.h"
#include <KDebug>

AdapterWidget::AdapterWidget(QString ubi, QWidget* parent) : Ui_AdapterWidget(),m_ubi(ubi)
{
	setupUi(this);
	setParent(parent);

	iface = new Solid::Control::BluetoothInterface(ubi);
	
	timeoutLabel->setEnabled(false);
	timeoutHorizontalSlider->setEnabled(false);

	// Set non-translated datas for the combo box items for further matching in slots
	modeComboBox->setItemData(0, QVariant(QString("hidden")));
	modeComboBox->setItemData(1, QVariant(QString("discoverable")));

	connect(modeComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(slotModeChanged(int)));
	connect(timeoutHorizontalSlider,SIGNAL(sliderMoved(int)),this,SLOT(slotSliderMoved(int)));
	connect(nameLineEdit,SIGNAL(editingFinished()),this,SLOT(slotNameChanged()));

	//connect(iface,SIGNAL(discoverableTimeoutChanged(int)),this,SLOT(setDiscoverableTimeout(int)));
	//connect(iface,SIGNAL(nameChanged(const QString&)),this,SLOT(setName(QString)));
}

AdapterWidget::~AdapterWidget()
{
}

void AdapterWidget::setMac(QString mac)
{
	macLabel->setText(mac);
	m_mac = mac;
}

QString AdapterWidget::getMac()
{
	return m_mac;
}

QString AdapterWidget::getVersion()
{
	return m_version;
}
void AdapterWidget::setName(QString name)
{
	if (nameLineEdit->text() != name) {
		nameLineEdit->setText(name);
		m_name = name;
	}
}

QString AdapterWidget::getName()
{
	return m_name;
}

void AdapterWidget::setDiscoverable(bool mode)
{
	if (mode)
		modeComboBox->setCurrentIndex(1);
	else
		modeComboBox->setCurrentIndex(0);
}

QString AdapterWidget::getMode()
{
	return modeComboBox->currentText();
}

void AdapterWidget::setDiscoverableTimeout(int timeout)
{
	if (timeout == 0)
		timeout = 1860;

	timeoutHorizontalSlider->setValue(timeout);
	if (timeout == 1860)
		timeoutLabel->setText("none");
	else
		timeoutLabel->setText(QString::number(timeout/60) + "min");
}

int AdapterWidget::getDiscoverableTimeout()
{
	return timeoutHorizontalSlider->value();
}

void AdapterWidget::slotModeChanged(int index)
{
	bool discoverable = false;

	if (modeComboBox->itemData(index) == "discoverable") {
		timeoutHorizontalSlider->setEnabled(true);
		timeoutLabel->setEnabled(true);
                discoverable = true;
	} else {
		timeoutHorizontalSlider->setEnabled(false);
		timeoutLabel->setEnabled(false);
	}
	
//	iface->setMode((Solid::Control::BluetoothInterface::Mode)mode);
	iface->setDiscoverable(discoverable);
}
/*
void AdapterWidget::slotModeChanged(Solid::Control::BluetoothInterface::Mode mode)
{
//	slotModeChanged(mode);
}
*/

void AdapterWidget::slotSliderMoved(int val)
{
	timeoutLabel->setText(QString::number(val/60) + "min");
	setDiscoverableTimeout(val);
	if (val >= 1860)
		iface->setDiscoverableTimeout(0);
	else
		iface->setDiscoverableTimeout(val);
}

void AdapterWidget::setDetails(QString rev, QString manu)
{
	QString msg = ( "\n<b>MAC: </b>" + m_mac + \
			"\n<b>Version: </b>" + m_version + \
			"\n<b>Revision: </b>" + rev + \
			"\n<b>Manufactor: </b>" + manu);
	infoGroupBox->setToolTip(msg);
}
/*
void AdapterWidget::setMinorClass(QString minorClass)
{
	int index = classComboBox->findText(minorClass);
	classComboBox->setCurrentIndex(index);

}
*/

void AdapterWidget::slotNameChanged()
{
	iface->setName(nameLineEdit->text());
	m_name = nameLineEdit->text();
	emit nameFieldChanged(this,m_name);
}

#include "moc_adapterwidget.cpp"