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

#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H


#include <QtGui>
#include <QDialog>

#include "ui_authdialog.h"


class AuthDialog : 	public QDialog,
			private Ui_AuthDialog
{
Q_OBJECT

public:
	AuthDialog(QObject* parent = 0);
	~AuthDialog();

	void setAddr(QString);
	void setName(QString);
	void setService(QString);
private:
	QObject* m_parent;

public slots:
	void slotTrustButton();
	void slotCancelButton();
	void slotAlwaysTrust();
	



};


#endif
