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

#include "wizarddialog.h"

#include <KApplication>
#include <QDebug>

#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <solid/control/bluetoothmanager.h>

static const KLocalizedString  description = ki18n("Bluetooth management software");
static const char *version = "0.4";

int main(int argc, char *argv[])
{
	KAboutData aboutData("kbluetooth-inputwizard",
				"kbluetooth",
				ki18n("KBluetooth"),
				"0.4",
				description,
				KAboutData::License_GPL,
				ki18n("(c) 2008-2009, The KBluetooth Developers")
				);
	aboutData.addAuthor(ki18n("Alex Fiestas"), ki18n("Maintainer"), "alex@eyeos.org", "http://www.afiestas.org/");
	aboutData.addAuthor(ki18n("Tom Patzig"), ki18n("Former Developer"), "tpatzig@suse.de", "http://www.kde.org/");

	qDebug() << "Bluetooth Adapters";

	//KCmdLineArgs::init( argc, argv, &aboutData );
	//KUniqueApplication::addCmdLineOptions();

	Solid::Control::BluetoothManager& btmanager = Solid::Control::BluetoothManager::self();
	if (btmanager.bluetoothInterfaces().size() == 0) {
		qDebug() << "No Bluetooth Adapter found!\n";
		return 0;
	}

	KCmdLineArgs::init(argc, argv, &aboutData);
	KApplication a;

	a.setQuitOnLastWindowClosed( true );

	BTWizard wiz(&a);
	wiz.show();

	return a.exec();
}
