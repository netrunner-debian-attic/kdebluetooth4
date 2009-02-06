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

#include "passkeydialog.h"

PasskeyDialog::PasskeyDialog(QObject* parent) : Ui_PasskeyDialog()
{
	m_parent = parent;
	setupUi(this);
	setWindowIcon(KIcon("kbluetooth4"));
	iconLabel->setPixmap(KIcon("kbluetooth4").pixmap(48,48));
	connect(okButton,SIGNAL(clicked()),this,SLOT(slotOkClicked()));
	connect(cancelButton,SIGNAL(clicked()),this,SLOT(slotCancelClicked()));


}

PasskeyDialog::~PasskeyDialog()
{


}

void PasskeyDialog::setName(QString name)
{

	nameLabel->setText(name);

}

void PasskeyDialog::setAddr(QString addr)
{

	addrLabel->setText(addr);

}

QString PasskeyDialog::getPin()
{

	return pinLineEdit->text();
}

void PasskeyDialog::slotOkClicked()
{
	emit okClicked();
	done(1);
}

void PasskeyDialog::slotCancelClicked()
{
	done(0);
}

void PasskeyDialog::clearPinCode()
{
    pinLineEdit->setText("");
}
