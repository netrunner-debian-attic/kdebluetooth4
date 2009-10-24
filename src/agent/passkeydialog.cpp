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

#include "passkeydialog.h"

PasskeyDialog::PasskeyDialog(QObject* parent)
{
	passkey = Ui_PasskeyDialog();
	m_parent = parent;
	passkey.setupUi(this);
	setWindowIcon(KIcon("kbluetooth"));
	passkey.iconLabel->setPixmap(KIcon("kbluetooth").pixmap(48,48));
	connect(passkey.okButton,SIGNAL(clicked()),this,SLOT(slotOkClicked()));
	connect(passkey.cancelButton,SIGNAL(clicked()),this,SLOT(slotCancelClicked()));
}

PasskeyDialog::~PasskeyDialog()
{
}

void PasskeyDialog::setName(QString name)
{
	passkey.nameLabel->setText(name);
}

void PasskeyDialog::setAddr(QString addr)
{
	passkey.addrLabel->setText(addr);
}

QString PasskeyDialog::getPin()
{
	return passkey.pinLineEdit->text();
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
	passkey.pinLineEdit->setText("");
}