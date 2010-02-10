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
#include "kdeversion.h"
#include "ui_receiveDialogWidget.h"
#include "obexserver/obexserversessionfiletransfer.h"
#include "config-nepomuk.h"

#include <QDesktopServices>
#include <QClipboard>

#if KDE_IS_VERSION(4,3,73)
	#include <KStatusNotifierItem>
	typedef KStatusNotifierItem KNotificationItem;
#else
	#include <knotificationitem-1/knotificationitem.h>
#endif

#ifdef HAVE_NEPOMUK
	#include <Nepomuk/ResourceManager>
	#include <Nepomuk/Resource>
	#include <Nepomuk/Tag>
	#include <Soprano/Vocabulary/RDFS>
	#include <Soprano/Vocabulary/NAO>
#endif

#include <kstatusbarjobtracker.h>
#include <KIO/JobUiDelegate>
#include <KIconLoader>
#include <KIconEffect>
#include <KFileDialog>
#include <KProcess>
#include <KTemporaryFile>
#include <KDirSelectDialog>
#include <KDialog>
#include <KUrlRequester>
#include <KDebug>
#include <KMenu>
#include <KApplication>
#include <KStandardAction>
#include <kwallet.h>

using KWallet::Wallet;
KBlueTray::KBlueTray(const QString& path, QObject* parent) : KNotificationItem(parent), agentPath(path),m_aboutDialog(0)
{
	adapter = 0;
	server = 0;
	serversession = 0;
	agent = 0;
	adapterConfig = 0;
	session = 0;
	progress = 0;
	devSelector = 0;
	lockSelector = 0;
	m_kBlueLock = 0;
	m_wallet = 0;
	modes << "Off" << "Discoverable" << "Connectable" ;
	
	setParent(kapp);
	setIconByName("kbluetooth");
	setToolTip("kbluetooth", "KBluetooth", "KDE bluetooth framework");

    setCategory(KNotificationItem::Hardware);
	setStandardActionsEnabled(false);//We do not want restore :(
	setAssociatedWidget(contextMenu());
	//If the object already exists or can't be created, at least kdebug it
	if(!QDBusConnection::systemBus().registerObject(agentPath, kapp)){
		kDebug() << "The dbus object can't be registered";
	}

	initIcons();
	initConfig();
	initMenu();

	Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();

	#warning kdevelop says that this dbus is never used
	dbus = new QDBusConnection("dbus");

	connect(&man,SIGNAL(interfaceAdded(const QString&)),this,SLOT(adapterAdded(const QString&)));
	connect(&man,SIGNAL(interfaceRemoved(const QString&)),this,SLOT(adapterRemoved(const QString&)));
	connect(&man,SIGNAL(defaultInterfaceChanged(const QString&)),this,SLOT(defaultAdapterChanged(const QString&)));

	kblueLockEnabled = false;

	if ( man.bluetoothInterfaces().size() > 0 ) {
		defaultAdapterUBI = man.defaultInterface();
		onlineMode();
		if ( config->group("KBlueLock").readEntry("LockEnabled",false) && ! config->group("KBlueLock").readEntry("Device","").isEmpty() ) {
			lockEnableAction->setChecked(true);
			m_kBlueLock = new KBlueLock(adapter);
			lockDevice = m_kBlueLock->lockDevice();
			connect(m_kBlueLock, SIGNAL(lockDisabled()), this, SLOT(lockDisabled()));
			connect(m_kBlueLock, SIGNAL(lockReady()), this, SLOT(lockReady()));
			m_kBlueLock->enable();
			lockEnabled();
		}
	} else {
		offlineMode();
	}
}

KBlueTray::~KBlueTray()
{
	delete agent;

	delete session;
	delete progress;
// 		delete devSelector;
// 		delete lockSelector;

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
	
	delete confGroup;
	delete config;
	
	delete m_kBlueLock;
	
	delete m_aboutDialog;
	qDebug() << "bye bye";
}

void KBlueTray::initIcons() {
	KIconLoader loader;
	QString iconPath = KIconLoader::global()->iconPath( "kbluetooth", KIconLoader::Panel );
	m_IconEnabled = QIcon( iconPath );
	QImage iconDisabled = m_IconEnabled.pixmap( loader.currentSize( KIconLoader::Panel ) ).toImage();
	KIconEffect::toGray( iconDisabled, 1.0 );
	m_IconDisabled = QIcon( QPixmap::fromImage( iconDisabled ) );
}

void KBlueTray::initMenu() {
	kbluelockMenu = contextMenu()->addMenu(KIcon("preferences-desktop-user-password"), i18n("KBlueLock"));

	lockEnableAction = kbluelockMenu->addAction(KIcon("system-lock-screen"), i18n("Enable"), this, SLOT(enableLock()));
	lockEnableAction->setCheckable(true);
	lockEnableAction->setChecked(false);

	lockConfigureAction = kbluelockMenu->addAction(KIcon("configure"), i18n("Configure..."), this, SLOT(slotConfigureKBlueLock()));
	lockConfigureAction->setDisabled(true);

	m_sendMenu = contextMenu()->addMenu(KIcon("text-directory"), i18n("Send"));

	sendToAction = m_sendMenu->addAction(KIcon("text-directory"), i18n("File"), this, SLOT(sendFile()));
	sendToAction->setDisabled(false);

	m_sendClipboardText = m_sendMenu->addAction(KIcon("edit-paste"), i18n("Clipboard Text"), this, SLOT(sendClipboardText()));

	wizardAction = contextMenu()->addAction(KIcon("kbluetooth"), i18n("Device Manager"), this, SLOT(showManager()));
	wizardAction->setDisabled(false);

	settingsMenu = contextMenu()->addMenu(KIcon("preferences-system"), i18n("Settings"));

	receivedAction = settingsMenu->addAction(KIcon("folder-documents"), i18n("Received files"), this, SLOT(receivedConfig()));
	adapterAction = settingsMenu->addAction(KIcon("configure"), i18n("Bluetooth Adapters"), this, SLOT(showAdapterInterface()));
	serverAction = settingsMenu->addAction(KIcon("network-server"), i18n("Obex server"), this, SLOT(enableServer()));
	serverAction->setCheckable(true);
	KConfigGroup obexServerConfig(config, "ObexServer");
	if(obexServerConfig.readEntry("Autostart") == "true") {
		kDebug() << "Enabling obex server";
		serverAction->setChecked(true);
	}

	aboutAction = contextMenu()->addAction(KIcon("folder-remote"), i18n("About"), this, SLOT(showAboutInfo()));
	contextMenu()->addSeparator();
	contextMenu()->addAction((QAction*)KStandardAction::quit(this,SLOT(slotQuitApp(bool)),this));
}

void KBlueTray::initConfig() {
	config = new KConfig("kbluetoothrc");
	confGroup = new KConfigGroup(config, "General");
	KConfigGroup obexServerConfig(config, "ObexServer");
	if(!obexServerConfig.hasKey("savePath")) {
		QString saveUrl;
		KDirSelectDialog dirs;
		if(dirs.exec() && dirs.url().isValid()) {
			saveUrl = dirs.url().path();
		}else{
			saveUrl = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
		}
		obexServerConfig.writeEntry("savePath",saveUrl);
		obexServerConfig.sync();
	}
	if(!obexServerConfig.hasKey("Autostart")) {
		obexServerConfig.writeEntry("Autostart", "true");
		obexServerConfig.sync();
	}
	if(!config->group("KBlueLock").hasKey("Device")) {
		config->group("KBlueLock").writeEntry("Device", "");
		config->group("KBlueLock").sync();
	}
	if(!config->group("KBlueLock").hasKey("LockEnabled")) {
		config->group("KBlueLock").writeEntry("LockEnabled", false);
		config->group("KBlueLock").sync();
	}
	if(!config->group("KBlueLock").hasKey("UnlockEnabled")) {
		config->group("KBlueLock").writeEntry("UnlockEnabled", true);
		config->group("KBlueLock").sync();
	}
}

void KBlueTray::onlineMode()
{
	kDebug() << "online Mode";
	Solid::Control::BluetoothManager &man = Solid::Control::BluetoothManager::self();
	kDebug() << "adapter size " << Solid::Control::BluetoothManager::self().bluetoothInterfaces().size();

	online = true;
    setStatus(KNotificationItem::Active);

	setIconByPixmap( m_IconEnabled );

	adapter = new Solid::Control::BluetoothInterface(man.defaultInterface());
	connect(adapter,SIGNAL(propertyChanged(const QString&, const QVariant&)),this,SLOT(slotPropertyChanged(const QString&, const QVariant&)));

	kDebug() << "Adapter found " << adapter->name();

	adapter->registerAgent(agentPath,"DisplayYesNo");
	if (!agent)
		agent = new Agent(kapp,adapter);
		agent->setBluetoothInterface(adapter);
		agent->setExitOnRelease(false);

	updateTooltip();
	
	sendToAction->setEnabled(true);
	wizardAction->setEnabled(true);
	adapterAction->setEnabled(true);
	kbluelockMenu->setEnabled(true);
	settingsMenu->setEnabled(true);
	m_sendClipboardText->setEnabled(true);
	m_sendMenu->setEnabled(true);

	if(serverAction->isChecked()) {
		if (server) {
			KConfigGroup obexServerConfig(config, "ObexServer");
			server->start(config->group("ObexServer").readEntry("savePath"), true, false);
		} else {
			createObexServer();
		}
	}
}

void KBlueTray::createObexServer()
{
	//TODO: Make this error beatiful
	server = new ObexServer(this, adapter->address(), "opp", false);
	connect(server, SIGNAL(openObexError()), this, SLOT(openObexError()));
	connect(server, SIGNAL(serverReady()), this, SLOT(slotServerReady()));
}

void KBlueTray::offlineMode()
{
	kDebug() << "offline Mode";

	online = false;

	sendToAction->setEnabled(false);
	wizardAction->setEnabled(false);
	adapterAction->setEnabled(false);
	kbluelockMenu->setEnabled(false);
	settingsMenu->setEnabled(false);
	m_sendClipboardText->setEnabled(false);
	m_sendMenu->setEnabled(false);

    setStatus(KNotificationItem::Passive);

	setIconByPixmap( m_IconDisabled );

	if(server) {
		server->close();
		connect(server,SIGNAL(closed()),this,SLOT(obexServerClosed()));
	}

	if(serversession) {
		serversession->cancel();
		serversession->disconnect();
		delete serversession;
		serversession = 0;
	}

}

void KBlueTray::obexServerClosed()
{
	delete server;
	server = 0;
}

void KBlueTray::updateTooltip() {
	kDebug() << "Updating Tooltip";	
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
	
	//tray->setToolTip(toolTipInfo);
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
	if(!m_aboutDialog) {
		m_aboutDialog = new KAboutApplicationDialog(KGlobal::mainComponent().aboutData());
		m_aboutDialog->exec();
	}
	
	if(m_aboutDialog->isVisible()){
		m_aboutDialog->raise();
	}else{
		m_aboutDialog->show();
	}
}

void KBlueTray::showAdapterInterface()
{
	if (!adapterConfig) {
		adapterConfig = new AdapterConfig(this);
		connect(adapterConfig,SIGNAL(adapterConfigClosed()),this,SLOT(updateTooltip()));
	}
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
	process.setProgram("kbluetooth-devicemanager");
	process.startDetached();

//	manager.exec();
}

//  ################# SEND FILE ################################

void KBlueTray::sendFile()
{
	QStringList filesNames = KFileDialog::getOpenFileNames(KUrl("./"), "*", 0, i18n("Select File"));
	if (!filesNames.isEmpty()) {
		//We're going to asume that right now the sendFile action is going on
		sendToAction->setDisabled(true);
		m_sendClipboardText->setDisabled(true);
		filesToSend = filesNames;
		devSelector = new DeviceSel(this,QString("computer,phone").split(','));
		devSelector->hideExtraCheckBox();
		devSelector->setInfoLabel(i18n("Selecting a device starts the file transfer."));
		connect(devSelector,SIGNAL(deviceSelected(const QString&)),this,SLOT(slotSendFile(const QString&)));
		connect(devSelector,SIGNAL(selectorCancelled()),this,SLOT(sendOpenObexError()));
	}
}

void KBlueTray::slotSendFile(const QString& mac)
{
	kDebug() << filesToSend;
	fileToSend = filesToSend.first();
	filesToSend.removeFirst();
	sendingToMac = mac;
	session = new ObexSession(this,adapter->address(),mac,"opp");
	//TODO: handle this beatiful
	if(session->error == true){
		sendToAction->setDisabled(false);
		m_sendClipboardText->setDisabled(false);
		return;
	}
	kDebug() << filesToSend;
	connect(session,SIGNAL(openObexError()),this,SLOT(sendOpenObexError()));
	connect(session,SIGNAL(connected()),this,SLOT(obexSessionReady()));
}

void KBlueTray::sendClipboardText()
{
	sendToAction->setDisabled(true);
	m_sendClipboardText->setDisabled(true);
	devSelector = new DeviceSel(this, QString("computer,phone").split(','));
	devSelector->hideExtraCheckBox();
	devSelector->setInfoLabel(i18n("Selecting a device starts the file transfer."));
	connect(devSelector, SIGNAL(deviceSelected(QString)), this, SLOT(slotSendClipboardText(QString)));
	connect(devSelector, SIGNAL(selectorCancelled()), this, SLOT(sendOpenObexError()));
}

void KBlueTray::slotSendClipboardText(const QString& address)
{
	KTemporaryFile tempFile;
	tempFile.setAutoRemove(false);
	QTextStream stream(&tempFile);
	if(tempFile.open()) {
		stream << QApplication::clipboard()->text();
		kDebug() << "Clipboard file: " << tempFile.fileName();
		filesToSend = QStringList(tempFile.fileName());
		tempFile.close();
		slotSendFile(address);
	} else {
		kWarning() << "Can't create a temporary file to send the clipboard text";
		sendToAction->setDisabled(false);
		m_sendClipboardText->setDisabled(false);
	}
}

void KBlueTray::obexSessionReady()
{
	if(session->error == false){
		//this is jut to be sure, in 0.5 we'll remove that and add multiple send support
		sendToAction->setDisabled(true);
		m_sendClipboardText->setDisabled(true);
		KJob* fileTransfer = session->sendFile(fileToSend);
		connect(fileTransfer, SIGNAL( result(KJob*) ), this, SLOT( slotFileTransferCompleted(KJob*) ));
		KIO::getJobTracker()->registerJob(fileTransfer);
		fileTransfer->start();
	}else{
		sendToAction->setDisabled(false);
		m_sendClipboardText->setDisabled(false);
	}
}

void KBlueTray::slotFileTransferCompleted(KJob* ) {
	kDebug() << filesToSend;
	if(!filesToSend.isEmpty())
	{
		fileToSend = filesToSend.first();
		filesToSend.removeFirst();
		obexSessionReady();
	}else{
		kDebug() << "Calling disconnect";
		connect(session,SIGNAL(disconnected()),this,SLOT(fileTransferFinal()));
		session->disconnect();
	}
}

void KBlueTray::fileTransferFinal()
{
	kDebug() << "Deleting session";
	delete session;
	session = 0;
	delete devSelector;
	devSelector = 0;
	sendToAction->setDisabled(false);
	m_sendClipboardText->setDisabled(false);
}

// ######################################### KBlueLock #####################################################

void KBlueTray::enableLock()
{
	if (lockEnableAction->isChecked() ) {
		m_kBlueLock = new KBlueLock(adapter);
		lockDevice = m_kBlueLock->lockDevice();
		if (lockDevice.isEmpty()) {
			kDebug() << "Configuring new KBlueLock Device";
			lockSelector = new DeviceSel(this,QString("computer,phone").split(','));
			lockSelector->setOkButtonText(i18n("Select"));
			lockSelector->setInfoLabel(i18n("Selecting a device enables KBlueLock (a balloon message will pop up from the tray when your target device is found and KBlueLock is ready).\nIf the device disappears, your screen will be locked."));

			connect(lockSelector, SIGNAL(deviceSelected(QString)), m_kBlueLock, SLOT(enable(QString)));
			connect(lockSelector,SIGNAL(selectorCancelled()),this,SLOT(slotBlueSelectorCancelled()));
			connect(lockSelector, SIGNAL(unlockChanged(bool)), m_kBlueLock, SLOT(unlockEnable(bool)));

			connect(m_kBlueLock, SIGNAL(lockEnabled()), this, SLOT(lockEnabled()));
			connect(m_kBlueLock, SIGNAL(lockDisabled()), this, SLOT(lockDisabled()));
			connect(m_kBlueLock, SIGNAL(lockReady()), this, SLOT(lockReady()));
		} else {
			connect(m_kBlueLock, SIGNAL(lockDisabled()), this, SLOT(lockDisabled()));
			connect(m_kBlueLock, SIGNAL(lockReady()), this, SLOT(lockReady()));
			m_kBlueLock->enable();
			lockEnabled();
		}
	} else {
		m_kBlueLock->disable();
	}
	updateTooltip();
}

void KBlueTray::lockEnabled()
{
	QString iconPath = KIconLoader::global()->iconPath( "document-encrypt", KIconLoader::Panel );
	setOverlayIconByPixmap( QIcon(iconPath).pixmap(14, 14) );
	setIconByPixmap(m_IconEnabled);
	showMessage("KBluetooth", "kbluelock is looking for the target device...", "kbluetooth");
	lockConfigureAction->setEnabled(true);
}

void KBlueTray::lockDisabled()
{
  	lockConfigureAction->setEnabled(false);
	kDebug() << "Lock disabled";
	setOverlayIconByPixmap(QIcon());
	setIconByPixmap(m_IconEnabled);
	delete m_kBlueLock;
	m_kBlueLock = 0;
}

void KBlueTray::lockReady()
{
	showMessage("KBluetooth", "kbluelock is enabled now", "kbluetooth");
}

void KBlueTray::slotConfigureKBlueLock()
{
		m_kBlueLock->disable();
		kDebug() << "Periodic Discovery Stopped";
		
		m_kBlueLock = new KBlueLock(adapter);
		m_kBlueLock->unlockEnable(true);

		lockSelector = new DeviceSel(this,QString("computer,phone").split(','));
		lockSelector->setOkButtonText("Select");
		lockSelector->setInfoLabel(i18n("Selecting a device enables KBlueLock (a balloon message will pop up from the tray when your target device is found and KBlueLock is ready).\nIf the device disappears, your screen will be locked."));
		
		connect(lockSelector, SIGNAL(deviceSelected(QString)), m_kBlueLock, SLOT(enable(QString)));
		connect(lockSelector,SIGNAL(selectorCancelled()),this,SLOT(slotBlueSelectorCancelled()));
		connect(lockSelector, SIGNAL(unlockChanged(bool)), m_kBlueLock, SLOT(unlockEnable(bool)));
		
		connect(m_kBlueLock, SIGNAL(lockEnabled()), this, SLOT(lockEnabled()));
		connect(m_kBlueLock, SIGNAL(lockDisabled()), this, SLOT(lockDisabled()));
		connect(m_kBlueLock, SIGNAL(lockReady()), this, SLOT(lockReady()));
}

void KBlueTray::slotBlueSelectorCancelled()
{
	disconnect(lockSelector, SIGNAL(deviceSelected(QString)), m_kBlueLock, SLOT(enable(QString)));
	disconnect(lockSelector,SIGNAL(selectorCancelled()),this,SLOT(slotBlueSelectorCancelled()));
	disconnect(lockSelector, SIGNAL(unlockChanged(bool)), m_kBlueLock, SLOT(unlockEnable(bool)));
	disconnect(lockSelector,0,0,0);
	kblueLockEnabled = false;
	//enableBlueLock(lockDevice);
}

//################# external SIGNALS ####################################

void KBlueTray::slotPropertyChanged(const QString& prop, const QVariant& value)
{
	kDebug() << "Property Changed: " << prop << value;
	if (prop == "Name")
		updateTooltip();
}


//################### RECEIVE FILES #############################################

void KBlueTray::receivedConfig()
{
	KDialog *dialog = new KDialog();
	KUrlRequester *urlR = new KUrlRequester(config->group("ObexServer").readEntry("savePath"),dialog);
	urlR->setMode(KFile::Directory);
	dialog->setMainWidget(urlR);
	dialog->setCaption(i18n("Received files directory:"));
	dialog->setButtons(KDialog::Ok | KDialog::Cancel);
	dialog->setMinimumWidth(430);
	int response = dialog->exec();
	QString saveUrl = urlR->url().path();

	if(response == QDialog::Accepted && !saveUrl.isEmpty())
	{
		config->group("ObexServer").writeEntry("savePath",saveUrl);
		config->group("ObexServer").sync();
	}
	delete dialog;
	
}

void KBlueTray::openObexError() {
	delete server;
	server = 0;
	serverAction->setEnabled(false);
	return;
}

void KBlueTray::sendOpenObexError(){
	sendToAction->setDisabled(false);
	m_sendClipboardText->setDisabled(false);
}

void KBlueTray::slotServerReady() {
	if(config->group("ObexServer").readEntry("Autostart") == "true") {
		if(!server->isStarted()) {
			server->start(config->group("ObexServer").readEntry("savePath"), true, false);
		}
	}
	connect(server, SIGNAL(started()), this, SLOT(slotServerStarted()));
	connect(server, SIGNAL(stopped()), this, SLOT(slotServerStopped()));
	connect(server, SIGNAL(closed()), this, SLOT(slotServerClosed()));
	connect(server, SIGNAL(sessionCreated(const QString&)), this, SLOT(slotServerSessionCreated(const QString&)));
	connect(server, SIGNAL(sessionRemoved(const QString&)), this, SLOT(slotServerSessionRemoved(const QString&)));
	connect(server, SIGNAL(errorOccured(const QString&, const QString&)), this, SLOT(slotServerErrorOccured(const QString&, const QString&)));
	updateTooltip();
}

void KBlueTray::slotServerErrorOccured(const QString& error_name, const QString& error_message) {
	kDebug() << error_name << ": " << error_message;
// 	KMessageBox::error(0, error_name, error_message);
}

void KBlueTray::enableServer() {
	if(serverAction->isChecked()) {
		config->group("ObexServer").writeEntry("Autostart", "true");
		if(server) {
			kDebug() << "THIS SHOULD NOT HAPPEN!";
			slotServerReady();
		} else {
			createObexServer();
		}
	} else {
		config->group("ObexServer").writeEntry("Autostart", "false");
		if(server)
			server->close();
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
	kDebug() << path;
	serversession = new ObexServerSession(this, server, path);
	connect(serversession, SIGNAL(disconnected()), this, SLOT(slotServerTransferDisconnected()));
	connect(serversession, SIGNAL(transferStarted(KJob*)), this, SLOT(slotServerSessionTransferStarted(KJob*)));
	connect(serversession, SIGNAL(errorOccurred(const QString&, const QString&)), this, SLOT(slotServerErrorOccured(const QString&, const QString&)));
}

void KBlueTray::slotServerSessionRemoved(const QString& /*path*/) {
	if(serversession) {
		serversession->disconnect();
		delete serversession;
		serversession = 0;
	}
}

//TODO: Be able to choose the destination path
void KBlueTray::slotServerSessionTransferStarted(KJob* job) {
	ObexServerSessionFileTransfer* fileTransfer = static_cast<ObexServerSessionFileTransfer*>(job);
	kDebug() << "Going to check: " << fileTransfer->remoteAddr();
	if(isReceiveTrusted(fileTransfer->remoteAddr()) == false){
		kDebug() << "the device is not trusted, showing dialog";
		KUrl tmpUrl(fileTransfer->localPath());
		Ui::receiveFileWidget *receiveWidget = new Ui::receiveFileWidget;
		QWidget *mainWidget = new QWidget();
		receiveWidget->setupUi(mainWidget);
		receiveWidget->saveToUrl->setText(tmpUrl.directory());

		KDialog *dialog = new KDialog();
		dialog->setMainWidget(mainWidget);
		dialog->setCaption(i18n("Receive files over bluetooth?"));
		dialog->setButtons(KDialog::Ok | KDialog::Cancel);
		dialog->setFixedSize(380,105);
		int response = dialog->exec();

		if(response == QDialog::Rejected){
			QFile file(fileTransfer->localPath());
			if(!file.remove()) {
				kDebug() << "Error deleting file: " << fileTransfer->localPath();
			}
			fileTransfer->reject();
			serversession->cancel();
			serversession->disconnect();
			return;
		}
		if(receiveWidget->saveCheck->isChecked()){
			addReceiveTrustedDevice(fileTransfer->remoteName(),fileTransfer->remoteAddr());
		}
		QString saveUrl = receiveWidget->saveToUrl->text();
		if(saveUrl.isEmpty()){
			saveUrl.append(config->group("ObexServer").readEntry("savePath"));
		}
		saveUrl.append("/");
		saveUrl.append(tmpUrl.fileName());
		kDebug() << saveUrl;
		fileTransfer->setLocalPath(saveUrl);
	}
	kDebug() << "transfer started";
#ifdef HAVE_NEPOMUK
	connect(fileTransfer, SIGNAL( result(KJob*) ), this, SLOT( slotFileReceiveComplete(KJob*) ));
#endif
	KIO::getJobTracker()->registerJob(fileTransfer);
	fileTransfer->start();
//	kDebug() << "Remote addr: " << fileTransfer->remoteAddr();
	/*KNotification* notification = new KNotification("receiveFiles", 0, KNotification::Persistent);
	notification->setText("Recieve files over bluetooth?");
	notification->setPixmap( KIcon("kbluetooth4").pixmap(20, 20) );
	notification->setActions(QStringList() << "Yes" << "no");
	connect(notification, SIGNAL( activated(unsigned int) ), this, SLOT( slotAcceptFileRecieve(unsigned int) ));
	notification->sendEvent();*/
}

bool KBlueTray::initReceiveTrusting()
{
	//Already initialized
	if(m_wallet){
		kDebug() << "Wallet alreday initialized";
		return true;
	}
	m_wallet = Wallet::openWallet(Wallet::NetworkWallet(),0);
	if(m_wallet == NULL){
		return false;
	}
	m_wallet->hasFolder("kbluetooth4");
	if(!m_wallet->hasFolder("kbluetooth4")){
		if(!m_wallet->createFolder("kbluetooth4")){
			//TODO: Error reporting here
			return false;
		}
	}
	m_wallet->setFolder("kbluetooth4");
	return true;
}
//TODO: shall we use the bluez trusting list? or that is for other matters?
void KBlueTray::addReceiveTrustedDevice(const QString& remoteName, const QString& remoteAddr) {
	if(initReceiveTrusting()) {
		m_wallet->writeEntry(remoteAddr,remoteName.toAscii());
		m_wallet->sync();
	}
}

bool KBlueTray::isReceiveTrusted(const QString& remoteAddr) {
	kDebug() << "Checking trust: " << remoteAddr;
	if(initReceiveTrusting()) {
		if(m_wallet->hasEntry(remoteAddr)) {
			kDebug() << "\t\ttrue";
			return true;
		}
	}
	return false;
}

void KBlueTray::slotAcceptFileRecieve(unsigned int id) {
	if(id == 1) {
		//accept
	} else {
		//deny
	}
}

void KBlueTray::slotFileReceiveComplete(KJob* job) {
#ifdef HAVE_NEPOMUK
	ObexServerSessionFileTransfer* fileTransfer = static_cast<ObexServerSessionFileTransfer*>(job);
	if(Nepomuk::ResourceManager::instance()->init() == 0) {
		kDebug() << fileTransfer->localPath();
		Nepomuk::Resource file(QUrl(fileTransfer->localPath()));
		Nepomuk::Tag tag("bluetooth");
		tag.addSymbol("preferences-system-bluetooth");
		tag.setDescription("File received over bluetooth");
		file.addTag(tag);
	}
#endif
}

void KBlueTray::slotFileReceiveCancelled() {
	serversession->cancel();
	serversession->disconnect();
}

void KBlueTray::slotServerTransferDisconnected() {

}
