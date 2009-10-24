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

#include "confirmdialog.h"

ConfirmDialog::ConfirmDialog(QObject* parent) : Ui_ConfirmDialog()
{
	m_parent = parent;

	setupUi(this);
	setWindowIcon(KIcon("kbluetooth"));
	iconLabel->setPixmap(KIcon("kbluetooth").pixmap(32,32));
	connect(yesButton,SIGNAL(clicked()),this,SLOT(slotYesButton()));
	connect(noButton,SIGNAL(clicked()),this,SLOT(slotNoButton()));
}

ConfirmDialog::~ConfirmDialog()
{
}

void ConfirmDialog::setAddr(QString addr)
{
	addressLabel->setText(addr);
}

void ConfirmDialog::setName(QString name)
{
	nameLabel->setText(name);
}

void ConfirmDialog::setMode(QString mode)
{
	titleLabel->setText(i18n("Switch Bluetooth Adapter?"));
	confirmLabel->setText(i18n("Mode: "));
	valueLabel->setText(mode);
}

void ConfirmDialog::setPassKey(QString passkey)
{
	titleLabel->setText(i18n("Confirm passkey?"));
	confirmLabel->setText(i18n("PassKey: "));
	valueLabel->setText(passkey);
}

void ConfirmDialog::slotYesButton()
{
	done(1);
}

void ConfirmDialog::slotNoButton()
{
	done(0);
}