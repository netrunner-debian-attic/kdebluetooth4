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

#include "trayicon.h"
#include "agent/agent.h"

#include <QDebug>

#include <kuniqueapplication.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

static const KLocalizedString  description = ki18n("Bluetooth management software");
static const char *version = "0.4";

int main(int argc, char *argv[])
{
	KAboutData aboutData("kbluetooth",
				0,
				ki18n("KBluetooth"),
				"0.4",
				description,
				KAboutData::License_GPL,
				ki18n("(c) 2008-2009, The KBluetooth Developers")
				);
	aboutData.addAuthor(ki18n("Alex Fiestas"), ki18n("Maintainer"), "alex@eyeos.org", "http://www.afiestas.org/");
	aboutData.addAuthor(ki18n("Tom Patzig"), ki18n("Former Developer"), "tpatzig@suse.de", "http://www.kde.org/");

// 	KComponentData component(&aboutData);

	KCmdLineArgs::init( argc, argv, &aboutData );

	KUniqueApplication::addCmdLineOptions();


	if (!KUniqueApplication::start()) {
		qDebug() << "KBluetooth is already running!\n";
		return 0;
	}

	KUniqueApplication a;

	a.setQuitOnLastWindowClosed( false );

	const QString path = "/kbluetooth4_agent";
	/*
	QDBusConnection::systemBus().registerObject(path, &a);
	Agent* agent = new Agent(&a);
	agent->setExitOnRelease(false);
	*/

	KBlueTray main(path);

	return a.exec();
}
