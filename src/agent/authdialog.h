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

#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include "ui_authdialog.h"

#include <QDialog>

class AuthDialog : 	public QDialog, private Ui_AuthDialog
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
