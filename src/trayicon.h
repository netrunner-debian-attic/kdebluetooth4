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


#ifndef KBLUETRAY_H
#define KBLUETRAY_H

#include <kdeversion.h>
#include "adapterconfig.h"
#include "inputwizard/wizarddialog.h"
#include "device-manager/devicemanager.h"
#include "obexsession/obexsession.h"
#include "deviceselector/devicesel.h"
#include "obexserver/obexserver.h"
#include "obexserver/obexserversession.h"
#include "agent/agent.h"
#include "kbluelock.h"

#include <QSystemTrayIcon>

#include <KDebug>
#include <kdialog.h>
#include <kaboutapplicationdialog.h>
#include <kdirselectdialog.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <KConfig>
#include <KConfigGroup>
#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothsecurity.h>
#include <kwallet.h>

#if KDE_IS_VERSION(4,3,73)
	#include <KStatusNotifierItem>
	typedef KStatusNotifierItem KNotificationItem;
#else
    #include <knotificationitem-1/knotificationitem.h>
    using namespace Experimental;
#endif

class KBlueTray : public KNotificationItem
{
Q_OBJECT

public:
	KBlueTray(const QString& path, QObject* parent = 0);
	~KBlueTray();

private:
	QStringList modes;
	QString toolTipInfo;
	QSystemTrayIcon* tray;

	QIcon m_IconDisabled;
	QIcon m_IconEnabled;

	QMenu* mainmenu;

	QAction* quitAction;
	QAction* aboutAction;
	QAction* adapterAction;
	QAction* receivedAction;
	QAction* serverAction;
	QAction* wizardAction;
	QAction* sendToAction;
	QAction* lockConfigureAction;
	QAction* lockEnableAction;
	QMenu* kbluelockMenu;
	QMenu* settingsMenu;
	QMenu* m_sendMenu;

	KConfig* config;
	KConfigGroup* confGroup;

	AdapterConfig* adapterConfig;

	Solid::Control::BluetoothSecurity* secure;
	Solid::Control::BluetoothInterface* adapter;

	ObexSession* session;
	ObexServer* server;
	ObexServerSession* serversession;

	QString defaultAdapterUBI;
	bool online;

	DeviceSel* devSelector;
	DeviceSel* lockSelector;
	QString fileToSend;
	QStringList filesToSend;
	QProgressDialog* progress;

	qulonglong totalFileSize;
	QString lockDevice;
	QString lockDeviceName;
	bool kblueLockEnabled;
	bool kblueUnlockEnabled;
	QDateTime awaytime;

	QDBusConnection* dbus;
	QString agentPath;

	Agent* agent;

	KBlueLock* m_kBlueLock;
	QAction* m_sendClipboardText;
	KAboutApplicationDialog *m_aboutDialog;
	QString sendingToMac;
	KWallet::Wallet *m_wallet;
	void initIcons();
	void initMenu();
	void initConfig();
	bool initReceiveTrusting();
	void addReceiveTrustedDevice(const QString& remoteName,const QString& remoteAddr);
	bool isReceiveTrusted(const QString& remoteAddr);

public slots:
	void slotQuitApp(bool);
	void showAboutInfo();
	void showAdapterInterface();
	void showWizard();
	void showManager();
	void updateTooltip();

	void onlineMode();
	void offlineMode();

	void createObexServer();
	void slotPropertyChanged(const QString&, const QVariant&);
	void adapterAdded(const QString&);
	void adapterRemoved(const QString&);
	void defaultAdapterChanged(const QString&);

	void enableLock();
	void slotConfigureKBlueLock();
	void slotBlueSelectorCancelled();

	void obexSessionReady();
	void sendFile();
	void slotSendFile(const QString&);
	void sendClipboardText();
	void slotSendClipboardText(const QString&);
	void slotFileTransferCompleted(KJob*);
	void fileTransferFinal();

	void receivedConfig();
	void enableServer();
	void slotServerReady();
	void openObexError();
	void slotServerErrorOccured(const QString&, const QString&);
	void slotServerStarted();
	void slotServerStopped();
	void slotServerClosed();
	void slotServerSessionCreated(const QString&);
	void slotServerSessionRemoved(const QString&);
	//void slotServerSessionTransferStarted(const QString&, const QString&, qulonglong);
	void slotServerSessionTransferStarted(KJob*);
	void slotFileReceiveCancelled();
	//void slotServerSessionTransferCompleted();
	void slotFileReceiveComplete(KJob*);
	void slotServerTransferDisconnected();
	void slotAcceptFileRecieve(unsigned int);
	void sendOpenObexError();

private slots:
	void lockEnabled();
	void lockDisabled();
	void lockReady();
	void obexServerClosed();
};
#endif
