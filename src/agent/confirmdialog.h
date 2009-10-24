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

#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include "ui_confirmdialog.h"

#include <QDialog>

class ConfirmDialog :	public QDialog, private Ui_ConfirmDialog
{
Q_OBJECT

public:
	ConfirmDialog(QObject* parent = 0);
	~ConfirmDialog();

	void setAddr(QString);
	void setName(QString);
	void setMode(QString);
	void setPassKey(QString);
private:
	QObject* m_parent;

public slots:
	void slotYesButton();
	void slotNoButton();
};
#endif
