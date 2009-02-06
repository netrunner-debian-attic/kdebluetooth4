/*
 *
 *  KBluetooth4 - KDE4 Bluetooth Framework
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

#include <kuniqueapplication.h>
#include <iostream>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <QtDBus>

#include "trayicon.h"
#include "agent/agent.h"

int main(int argc, char *argv[])
{
	
static const char *description =
    I18N_NOOP("kbluetooth4");

	KAboutData aboutData("kbluetooth4",
               I18N_NOOP("KBluetooth4"),
               ki18n("KBluetooth4 - The KDE4 Bluetooth Framework"),"0.3",
       	ki18n(description), KAboutData::License_GPL);
	aboutData.addAuthor(ki18n("Tom Patzig"), KLocalizedString(), "tpatzig@suse.de", "http://www.kde.org/");

	
    KComponentData component(&aboutData);

    KCmdLineArgs::init( argc, argv, &aboutData );

    KUniqueApplication::addCmdLineOptions();

    
    if (!KUniqueApplication::start()) {
	fprintf(stderr, "KBluetooth4 is already running!\n");
        return 0;
    }

//  QApplication a(argc, argv);

    KUniqueApplication a;

    a.disableSessionManagement();
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
