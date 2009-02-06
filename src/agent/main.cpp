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


#include <QString>
#include <solid/control/bluetoothmanager.h>
#include "agent.h"

#include <kuniqueapplication.h>
#include <iostream>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <QtDBus>


int main(int argc, char *argv[])
{
	
static const char *description =
    I18N_NOOP("kbluetooth4-agent");

	KAboutData aboutData("kbluetooth4-agent",
               I18N_NOOP("KBluetooth4-Agent"),
               ki18n("KBluetooth4-Agent - The KDE4 Bluetooth Agent"),"0.1",
       	ki18n(description), KAboutData::License_GPL);
	aboutData.addAuthor(ki18n("Tom Patzig"), KLocalizedString(), "tpatzig@suse.de", "http://www.kde.org/");

	
    KComponentData component(&aboutData);

    KCmdLineArgs::init( argc, argv, &aboutData );

    KUniqueApplication::addCmdLineOptions();

    
    if (!KUniqueApplication::start()) {
	fprintf(stderr, "KBluetooth4-Agent is already running!\n");
        return 0;
    }

    const QString path = "/kbluetooth4_agent";

    KUniqueApplication a;

    QDBusConnection::systemBus().registerObject(path, &a);
    
    new Agent(&a, path);

    return a.exec();
}
