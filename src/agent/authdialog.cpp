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

#include "authdialog.h"

AuthDialog::AuthDialog(QObject* parent) : Ui_AuthDialog()
{
	m_parent = parent;
	
	setupUi(this);
	setWindowIcon(KIcon("kbluetooth4"));
	iconLabel->setPixmap(KIcon("kbluetooth4").pixmap(32,32));
	connect(trustButton,SIGNAL(clicked()),this,SLOT(slotTrustButton()));
	connect(cancelButton,SIGNAL(clicked()),this,SLOT(slotCancelButton()));
	connect(allwTrustButton,SIGNAL(clicked()),this,SLOT(slotAlwaysTrust()));

}

AuthDialog::~AuthDialog()
{


}

void AuthDialog::setAddr(QString addr)
{
	addressLabel->setText(addr);
}

void AuthDialog::setName(QString name)
{
	nameLabel->setText(name);
}

void AuthDialog::setService(QString service)
{
	serviceLabel->setText(service);
}

void AuthDialog::slotTrustButton()
{
	done(1);
}

void AuthDialog::slotCancelButton()
{
	done(0);
}
void AuthDialog::slotAlwaysTrust()
{
	done(2);
}
