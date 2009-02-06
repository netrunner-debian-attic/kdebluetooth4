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


#include "trayicon.h"

KBlueTray::KBlueTray(const QString& path, QObject* parent) : QObject(parent), agentPath(path)
{

	 modes << "Off" << "Discoverable" << "Connectable" ;
	 adapter = 0;
	 server = 0;
	 serversession = 0;
         agent = 0;
	 
	 setParent(kapp);

	 tray = new QSystemTrayIcon(this);
	 tray->setIcon(KIcon("kbluetooth4"));

	 mainmenu = new QMenu();

         QDBusConnection::systemBus().registerObject(agentPath, kapp);

//	 kbluelockMenu = mainmenu->addMenu(KIcon("system-lock-screen"),i18n("KBlueLock"));

/*       Currently no DeviceDisappeared Signal is thrown by Bluez

         kbluelockMenu = mainmenu->addMenu(KIcon("preferences-desktop-user-password"),i18n("KBlueLock"));
	 

	 lockEnableAction = kbluelockMenu->addAction(KIcon("system-lock-screen"),i18n("Enable"));
	 lockEnableAction->setCheckable(true);
	 lockEnableAction->setChecked(false);
	 connect(lockEnableAction, SIGNAL(triggered(bool)), this, SLOT(enableLock()));

	 lockConfigureAction = kbluelockMenu->addAction(KIcon("configure"), i18n("Configure..."));
	 connect(lockConfigureAction,SIGNAL(triggered(bool)), this, SLOT(slotConfigureKBlueLock()));
	 lockConfigureAction->setDisabled(true);
*/

	 sendToAction = mainmenu->addAction(KIcon("text-directory"), i18n("Send File"));
	 connect(sendToAction, SIGNAL(triggered(bool)), this, SLOT(sendFile()));
         sendToAction->setDisabled(false);

	 wizardAction = mainmenu->addAction(KIcon("kbluetooth4"), i18n("Device Manager"));
	 connect(wizardAction, SIGNAL(triggered(bool)), this, SLOT(showManager()));
         wizardAction->setDisabled(false);
	
	 settingsMenu = mainmenu->addMenu(KIcon("preferences-system"), i18n("Settings"));
	
	 adapterAction = settingsMenu->addAction(KIcon("configure"), i18n("Bluetooth Adapters"));
	 connect(adapterAction, SIGNAL(triggered(bool)), this, SLOT(showAdapterInterface()));
	 
	 serverAction = settingsMenu->addAction(KIcon("network-server"), i18n("Obex server"));
	 serverAction->setCheckable(true);
	 serverAction->setChecked(true);
	 connect(serverAction, SIGNAL(triggered(bool)), this, SLOT(enableServer()));

	 aboutAction = mainmenu->addAction(KIcon("folder-remote"),i18n("About"));
	 connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(showAboutInfo()));

	 quitAction = mainmenu->addAction(KIcon("application-exit"),i18n("Quit"));
	 connect(quitAction, SIGNAL(triggered(bool)), this, SLOT(slotQuitApp(bool)));
	 
	 tray->setContextMenu(mainmenu);

	 Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();
         dbus = new QDBusConnection("dbus");

	 connect(&man,SIGNAL(interfaceAdded(const QString&)),this,SLOT(adapterAdded(const QString&)));
	 connect(&man,SIGNAL(interfaceRemoved(const QString&)),this,SLOT(adapterRemoved(const QString&)));
	 connect(&man,SIGNAL(defaultInterfaceChanged(const QString&)),this,SLOT(defaultAdapterChanged(const QString&)));

	 config = new KConfig("kbluetoothrc");
	 confGroup = new KConfigGroup(config,"General");
	 
	 QDir dir(QDir::homePath());
	 dir.mkdir(".kbluetooth4");

/*
	 if (confGroup->exists()) {
		avahiClient->receiver->setTargetPath(confGroup->readEntry("Path",avahiClient->receiver->getTargetPath()));
		avahiClient->setUserName(confGroup->readEntry("Nick",avahiClient->getUserName()));
	 }
*/

	//tray->setVisible(true);
	adapterConfig = 0;
	session = 0;
	progress = 0;
	devSelector = 0;
	lockSelector = 0;
	kblueLockEnabled = false;
	
	if ( man.bluetoothInterfaces().size() > 0 ) {
                defaultAdapterUBI = man.defaultInterface();
		onlineMode();
	} else {
		offlineMode();
	}
        
        

}

KBlueTray::~KBlueTray()
{

	if (adapter) {
                adapter->unregisterAgent(agentPath);
		adapter->stopDiscovery();
		delete adapter;
	}

	if (session)
		delete session;
	if (progress)
		delete progress;
	if (devSelector)
//		delete devSelector;
	if (lockSelector)
//		delete lockSelector;

	if(server) {
		server->stop();
		server->close();
		delete server;
	}
	
	if(serversession) {
		serversession->cancel();
		serversession->disconnect();
		delete serversession;
	}
	
	delete mainmenu;
	delete confGroup;
	delete config;
	delete tray;
	
	qDebug() << "bye bye";
}

void KBlueTray::onlineMode()
{
	kDebug() << "online Mode";
	Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();	
	kDebug() << "adapter size " << Solid::Control::BluetoothManager::self().bluetoothInterfaces().size();

	online = true;
	tray->setVisible(true);
	
	adapter = new Solid::Control::BluetoothInterface(man.defaultInterface());


	connect(adapter,SIGNAL(propertyChanged(const QString&, const QVariant&)),this,SLOT(slotPropertyChanged(const QString&, const QVariant&)));

	kDebug() << "Adapter found " << adapter->name();

        adapter->registerAgent(agentPath,"DisplayYesNo");
        agent = new Agent(kapp);
        agent->setExitOnRelease(false);
	
	updateTooltip();
	
	sendToAction->setEnabled(true);
	wizardAction->setEnabled(true);
	adapterAction->setEnabled(true);
//	kbluelockMenu->setEnabled(true);
        settingsMenu->setEnabled(true);
	serverAction->setChecked(true);

        if (!server) {
            server = new ObexServer(this, "00:00:00:00:00:00", "opp", true);
            connect(server, SIGNAL(serverReady()), this, SLOT(slotServerReady()));
            connect(server, SIGNAL(started()), this, SLOT(slotServerStarted()));
            connect(server, SIGNAL(stopped()), this, SLOT(slotServerStopped()));
            connect(server, SIGNAL(closed()), this, SLOT(slotServerClosed()));
            connect(server, SIGNAL(sessionCreated(const QString&)), this, SLOT(slotServerSessionCreated(const QString&)));
            connect(server, SIGNAL(sessionRemoved(const QString&)), this, SLOT(slotServerSessionRemoved(const QString&)));
            connect(server, SIGNAL(errorOccured(const QString&, const QString&)), this, SLOT(slotServerErrorOccured(const QString&, const QString&)));
        } else
            enableServer();


}

void KBlueTray::offlineMode()
{
	kDebug() << "offline Mode";

	online = false;

	sendToAction->setEnabled(false);
	wizardAction->setEnabled(false);
	adapterAction->setEnabled(false);
//	kbluelockMenu->setEnabled(false);
	settingsMenu->setEnabled(false);
	
	//tray->setToolTip("No Bluetooth Adapter");
	tray->setVisible(false);
		
	if (adapter) {
                kDebug() << "Unregistering Agent";
                adapter->unregisterAgent(agentPath);
		disconnect(adapter,0,0,0);
		delete adapter;
		adapter = 0;
	}

        if (agent) {
            delete agent;
            agent = 0;
        }
	
	if(server) {
		server->stop();
	}
	
	if(serversession) {
		serversession->cancel();
		serversession->disconnect();
		delete serversession;
		serversession = 0;
	}


}

void KBlueTray::updateTooltip() {
	
	if(!online) {
		toolTipInfo = i18n("No Bluetooth Adapter");
		return;
	}

        if(!adapter || !server ) 
            return;
	
	
	toolTipInfo = 	"<b>Name:</b> " + adapter->name() + "<br />" \
			+	"<b>Address:</b> " + adapter->address() + "<br />" \
			+	"<b>KBlueLock:</b> ";

	if (!kblueLockEnabled)
		toolTipInfo += "Disabled"; 
	else
		toolTipInfo += ("Enabled for " + lockDeviceName);
		
	toolTipInfo += "<br /><b>Receiving Files:</b> ";
	if(server) {
		toolTipInfo += ((server->isStarted()) ? "Enabled" : "Disabled");
	} else {
		toolTipInfo += "Disabled";
	}
	
	tray->setToolTip(toolTipInfo);
}

void KBlueTray::adapterAdded(const QString& ubi)
{
	 kDebug() << "adapterAdded: " << ubi;

}

void KBlueTray::adapterRemoved(const QString& ubi)
{
	 kDebug() << "adapterRemoved: " << ubi;
	 Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();
	 
	 if (man.bluetoothInterfaces().size() == 0) {
		offlineMode();
		kDebug() << "default Adapter removed";
	 }
	updateTooltip();
}

void KBlueTray::defaultAdapterChanged(const QString& ubi)
{
	kDebug() << "default Adapter changed " << ubi;
        if (!online) {
	    kDebug() << "new default adapter UBI: " << ubi;
    	    defaultAdapterUBI = ubi;
            onlineMode();
        }
}

void KBlueTray::slotQuitApp(bool /*set*/)
{
        kapp->quit();
}

void KBlueTray::showAboutInfo()
{
        KAboutData aboutData("kbluetooth4", 0, ki18n("KBluetooth4"), "0.3",
                           ki18n("KDE4 Bluetooth Framework"), KAboutData::License_GPL_V2);
        aboutData.addAuthor(ki18n("Tom Patzig"), KLocalizedString(), "tpatzig@suse.de", "http://www.kde.org/");
	aboutData.addCredit(ki18n("Arjan Topolovec"), ki18n(""), "arjan.top@gmail.com", "");
        aboutData.setProgramLogo(KIcon("kbluetooth4"));

        KAboutApplicationDialog about(&aboutData, mainmenu);
//        about.setIcon( KIcon( "kbluetooth4" ) );
        about.exec();
}

void KBlueTray::showAdapterInterface()
{
	if (!adapterConfig)
		adapterConfig = new AdapterConfig(this);

	adapterConfig->initialize();

}

void KBlueTray::showWizard()
{
//bluez4//	BTWizard wiz;
//bluez4//	wiz.exec();
}

void KBlueTray::showManager()
{
//	DeviceMan manager;
	kDebug() << "Starting Device Manager";
	KProcess process;
	process.setProgram("kbluetooth4-devicemanager");
	process.startDetached();
	
//	manager.exec();
}

//  ################# SEND FILE ################################

void KBlueTray::sendFile()
{
// bluez4//    
	QString fileName = KFileDialog::getOpenFileName(KUrl("./"),"*",mainmenu, tr("Select File"));
        if (!fileName.isEmpty()) {
		fileToSend = fileName;
		devSelector = new DeviceSel(this,QString("computer,phone").split(','));
	 	devSelector->hideExtraCheckBox();
		devSelector->setInfoLabel(i18n("Selecting a device starts the Filetransfer."));
		connect(devSelector,SIGNAL(deviceSelected(const QString&)),this,SLOT(slotSendFile(const QString&)));

        }


}

void KBlueTray::slotSendFile(const QString& mac)
{
	session = new ObexSession(this,adapter->address(),mac,"ftp");
	connect(session,SIGNAL(connected()),this,SLOT(obexSessionReady()));

}

void KBlueTray::obexSessionReady()
{
	kDebug() << "is connected: " << session->isConnected();

	session->sendFile(fileToSend);
	
	connect(session,SIGNAL(transferStarted(const QString&,const QString&,qulonglong)),this,SLOT(slotFileSendStarted(const QString,const QString&,qulonglong)));
	connect(session,SIGNAL(transferProgress(qulonglong)),this,SLOT(slotTransferProgress(qulonglong)));
	connect(session,SIGNAL(transferCompleted()),this,SLOT(slotFileSendFinished()));
	connect(session, SIGNAL(errorOccurred(const QString&, const QString&)), this, SLOT(slotServerErrorOccured(const QString&, const QString&)));

}

void KBlueTray::slotFileSendStarted(const QString& filename ,const QString& path ,qulonglong size)
{
	totalFileSize = size;

	kDebug() << "file send started: " << path << filename;
	kDebug() << "size: " << size;

	progress = new QProgressDialog(i18n("Sending ") + filename,i18n("Cancel"),0,100);
	progress->setAutoClose(true);
	progress->setWindowIcon(KIcon("kbluetooth4"));
	progress->show();
	connect(progress,SIGNAL(canceled()),this,SLOT(slotFileSendCanceled()));
	
}

void KBlueTray::slotTransferProgress(qulonglong val)
{
	kDebug() << "Transfered: " << val;

		
	qulonglong prog = ( ( val * 100 ) / totalFileSize) ;

	kDebug() << "Progress: " << prog;

	if(progress)
		  progress->setValue(prog);

}

void KBlueTray::slotFileSendFinished()
{
	kDebug() << "Transfer completed";

	session->disconnect();
	session->close();

	delete session;
	session = 0;

        if (progress) {
            delete progress;
            progress = 0;
        }
}

void KBlueTray::slotFileSendCanceled()
{
	session->cancel();
	session->disconnect();
	session->close();

	delete session;
	delete devSelector;

}

// ######################################### KBlueLock #####################################################

void KBlueTray::enableLock()
{
	if (lockEnableAction->isChecked() ) {
		if (lockDevice.isEmpty()) {
                        kDebug() << "Configuring new KBlueLock Device";
	 	 	kblueUnlockEnabled = true;
			lockSelector = new DeviceSel(this,QString("computer,phone").split(','));
			lockSelector->setOkButtonText(i18n("Select"));
			lockSelector->setInfoLabel(i18n("Selecting a device enables KBlueLock (a balloon message in the tray icon pops up when your target device is found and KBlueLock is ready).\nIf the device disappears your screen gets locked."));

			connect(lockSelector,SIGNAL(deviceSelected(const QString&)),this,SLOT(enableBlueLock(const QString&)));
			connect(lockSelector,SIGNAL(selectorCancelled()),this,SLOT(slotBlueSelectorCancelled()));
			connect(lockSelector,SIGNAL(unlockChanged(bool)),this,SLOT(slotUnlockScreenChanged(bool)));
		} else 
			enableBlueLock(lockDevice);

	} else {
		disableBlueLock();
	}
	updateTooltip();
}


void KBlueTray::enableBlueLock(const QString&  mac)
{
	disconnect(lockSelector,0,0,0);
	disconnect(adapter,SIGNAL(deviceDisappeared(const QString)),0,0);

	lockDevice = mac;
	if (!lockDevice.isEmpty()) {

            lockConfigureAction->setEnabled(true);

            kDebug() << "KBlueLock enabled";
            kDebug() << "lockdevice: " << lockDevice;

            adapter->startDiscovery();
            kDebug() << "Periodic Discovery Started";
            connect(adapter,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotTargetDeviceFound(const QString&, const QMap< QString,QVariant > & )));
            tray->showMessage("KBluetooth","kbluelock is looking for the target device...",
                    QSystemTrayIcon::Information,30000);

            connect(adapter,SIGNAL(deviceDisappeared(const QString&)),this,SLOT(slotValidateLockDev(const QString&)));

            kblueLockEnabled = false;

		
	} else {
		lockEnableAction->setChecked(false);
	}
}

void KBlueTray::disableBlueLock()
{

	kDebug() << "KBlueLock disabled";
	kDebug() << "lockdevice: " << lockDevice;
	
	updateTooltip();
	kblueLockEnabled = false;
	
	lockConfigureAction->setEnabled(false);

        adapter->stopDiscovery();
        kDebug() << "Periodic Discovery Stopped";

	disconnect(adapter,SIGNAL(deviceDisappeared(const QString&)),this,SLOT(slotValidateLockDev(const QString&)));
	disconnect(adapter,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotTargetDeviceFound(const QString&, const QMap< QString,QVariant > & )));

}

void KBlueTray::slotValidateLockDev(const QString& addr)
{
	if (addr == lockDevice) {
		
		kDebug() << "KBlueLock armed -- Lock Device disappeared";
		QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");
		QString service = "org.freedesktop.ScreenSaver";
	        QString path = "/ScreenSaver";
        	QString method = "Lock";
        	QString iface = "org.freedesktop.ScreenSaver";

        	QDBusInterface* interface = new QDBusInterface(service,path,iface,dbusconn);
        	interface->call(method);

	 	 	 // log the Away times ;)
		QFile file(QDir::homePath() + "/kbluelock.log");
	 	if (!file.open(QIODevice::Append | QIODevice::Text))
         		kDebug() << "cant open logfile";
	 	else {
			QTextStream out(&file);
	 	 	awaytime = QDateTime::currentDateTime();
			out << "Away from Computer: " << awaytime.toString() << "\n";
			file.flush();	 	 	
			file.close();
	 	}
		

	        connect(adapter,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotTargetDeviceBack(const QString&, const QMap< QString,QVariant > & )));

	}
}


void KBlueTray::slotConfigureKBlueLock()
{
                adapter->stopDiscovery();
                kDebug() << "Periodic Discovery Stopped";
			
		kblueLockEnabled = false;

		lockSelector = new DeviceSel(this,QString("computer,phone").split(','));
		lockSelector->setOkButtonText("Select");
		lockSelector->setInfoLabel(i18n("Selecting a device enables KBlueLock (a balloon message in the tray icon pops up when your target device is found and KBlueLock is ready).\nIf the device disappears your screen gets locked."));
		connect(lockSelector,SIGNAL(deviceSelected(const QString&)),this,SLOT(enableBlueLock(const QString&)));
		connect(lockSelector,SIGNAL(selectorCancelled()),this,SLOT(slotBlueSelectorCancelled()));
		connect(lockSelector,SIGNAL(unlockChanged(bool)),this,SLOT(slotUnlockScreenChanged(bool)));
}


void KBlueTray::slotBlueSelectorCancelled()
{

	disconnect(adapter,SIGNAL(deviceDisappeared(const QString&)),this,SLOT(slotValidateLockDev(const QString&)));
	disconnect(adapter,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotTargetDeviceFound(const QString&, const QMap< QString,QVariant > & )));
	disconnect(lockSelector,0,0,0);
	kblueLockEnabled = false;
	enableBlueLock(lockDevice);

}
void KBlueTray::slotTargetDeviceFound(const QString& addr, const QMap< QString,QVariant > & props )
{
	if ((!kblueLockEnabled) && (addr == lockDevice)) {
		kDebug() << "Target device found - Kbluelock ready";
                lockDeviceName = props["Name"].toString();
		tray->showMessage("KBluetooth","kbluelock is enabled now");
		kblueLockEnabled = true;
	        disconnect(adapter,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotTargetDeviceFound(const QString&, const QMap< QString,QVariant > & )));
		updateTooltip();
	}

}

void KBlueTray::slotTargetDeviceBack(const QString& addr, const QMap< QString,QVariant > & /* props */ )
{
	if ( (kblueLockEnabled && kblueUnlockEnabled ) && (addr == lockDevice)) {
		kDebug() << "Target device is back. Disabling Screensaver.";

		QDBusConnection dbusconn = dbus->connectToBus(QDBusConnection::SessionBus, "dbus");
		QString service = "org.freedesktop.ScreenSaver";
	        QString path = "/ScreenSaver";
       		QString method = "SetActive";
       		QString iface = "org.freedesktop.ScreenSaver";

       		QDBusInterface* interface = new QDBusInterface(service,path,iface,dbusconn);
       		interface->call(method,false);

			 // log the Away times ;)
		QFile file(QDir::homePath() + "/.kbluetooth4/kbluelock.log");
		if (!file.open(QIODevice::Append | QIODevice::Text))
         	 	 kDebug() << "cant open logfile";
	 	else {
			QTextStream out(&file);

			out << "Back on Computer: " << QDateTime::currentDateTime ().toString() << "\n";
	 	 	out << "---------------------------------------------\n";
	 	    	out << ">>> Pause: " << awaytime.secsTo(QDateTime::currentDateTime())/60 << " min ";
	 	    	out << awaytime.secsTo(QDateTime::currentDateTime())%60 << " sec\n\n";
			file.flush();	 	 	
			file.close();
	 	}
	
	        disconnect(adapter,SIGNAL(deviceFound( const QString&, const QMap< QString,QVariant > & )),this,SLOT(slotTargetDeviceBack(const QString&, const QMap< QString,QVariant > & )));
	}
}

//################# external SIGNALS ####################################

void KBlueTray::slotUnlockScreenChanged(bool val)
{
    kDebug() << "slotUnlockScreenChanged " << val;
    kblueUnlockEnabled = val;
}

void KBlueTray::slotPropertyChanged(const QString& prop, const QVariant& value)
{
    kDebug() << "Property Changed: " << prop << value;
    if (prop == "Name")
        updateTooltip();
}


//################### RECEIVE FILES #############################################

void KBlueTray::slotServerReady() {
	if(!server->isStarted())
		server->start(QDir::homePath()+"/.kbluetooth4", true, false);
}

void KBlueTray::slotServerErrorOccured(const QString& error_name, const QString& error_message) {
	kDebug() << error_name << ": " << error_message;
	KMessageBox::error(0, error_name, error_message);
}

void KBlueTray::enableServer() {
	if(serverAction->isChecked()) {
		slotServerReady();
	} else {
		server->stop();
	}
}

void KBlueTray::slotServerStarted() {
	if(!server->isStarted()) {
	      kDebug() << "obex server not started";
	      server->close();
	} else {
	      kDebug() << "obex server started";
	      serverAction->setChecked(true);
	}
	updateTooltip();
}

void KBlueTray::slotServerStopped() {
	serverAction->setChecked(false);
	updateTooltip();
	kDebug() << "obex server stopped";
}

void KBlueTray::slotServerClosed() {
	kDebug() << "obex server closed";
	serverAction->setChecked(false);
	updateTooltip();
	delete server;
	server = 0;
}

void KBlueTray::slotServerSessionCreated(const QString& path) {
	serversession = new ObexServerSession(this, path);
	connect(serversession, SIGNAL(disconnected()), this, SLOT(slotServerTransferDisconnected()));
	
//	KDirSelectDialog dlg(KUrl("."), false, mainmenu);
//	dlg.exec();
	
//	kDebug() << dlg.url().url();
	
	connect(serversession, SIGNAL(transferStarted(const QString&, const QString&, qulonglong)), this, SLOT(slotServerSessionTransferStarted(const QString&, const QString&, qulonglong)));
	connect(serversession, SIGNAL(transferProgress(qulonglong)), this, SLOT(slotTransferProgress(qulonglong)));
	connect(serversession, SIGNAL(transferCompleted()), this, SLOT(slotServerSessionTransferCompleted()));
	connect(serversession, SIGNAL(errorOccurred(const QString&, const QString&)), this, SLOT(slotServerErrorOccured(const QString&, const QString&)));
}

void KBlueTray::slotServerSessionRemoved(const QString& /*path*/) {
	if(progress) {
		 disconnect(progress, 0,0,0);
		 delete progress;
		 progress = 0;
	}
	if(serversession) {
		serversession->disconnect();
		delete serversession;
		serversession = 0;
	}
}

void KBlueTray::slotServerSessionTransferStarted(const QString& filename, const QString& local_path, qulonglong total_bytes) {
	

	int receiveFiles = KMessageBox::questionYesNo(mainmenu, "Receive files over bluetooth?", "Incoming files over Bluetooth");
        kDebug() << "Receive Files? " << receiveFiles;
	if(receiveFiles == 4) {
		serversession->reject();
		serversession->disconnect();
		return;
	}
        serversession->accept();

	kDebug() << local_path;
	kDebug() << total_bytes;
	
	totalFileSize = total_bytes;
	
	progress = new QProgressDialog(i18n("Receiving ") + filename, i18n("Cancel"), 0, 100);
	progress->setAutoClose(true);
	progress->setWindowIcon(KIcon("kbluetooth4"));
	progress->window()->setWindowTitle(i18n("Receiving ") + filename);
	progress->show();
	connect(progress, SIGNAL(canceled()), this, SLOT(slotFileReceiveCancelled()));
}

void KBlueTray::slotFileReceiveCancelled() {
	serversession->cancel();
	serversession->disconnect();
}

void KBlueTray::slotServerSessionTransferCompleted() {
	/*KDirSelectDialog dlg(KUrl("."), false, 0);
	dlg.exec();
	
	kDebug() << dlg.url().url();*/
	
	delete progress;
	progress = 0;
	
}

void KBlueTray::slotServerTransferDisconnected() {
	if(progress) {
		 delete progress;
		 progress = 0;
	}
}

