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


#ifndef KBLUETRAY_H
#define KBLUETRAY_H

#include <KDebug>
#include <KApplication>
#include <kdialog.h>
#include <kaboutapplicationdialog.h>
#include <kdirselectdialog.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <KConfig>
#include <KConfigGroup>
#include <KFileDialog>
#include <KIcon>
#include <KProcess>

#include <QSystemTrayIcon>
#include <QtGui>
#include <QtDBus>
#include <QMainWindow>
#include <QDir>

#include <solid/control/bluetoothmanager.h>
#include <solid/control/bluetoothsecurity.h>


#include "adapterconfig.h"
#include "inputwizard/wizarddialog.h"
#include "device-manager/devicemanager.h"
#include "obexsession/obexsession.h"
#include "deviceselector/devicesel.h"
#include "obexserver/obexserver.h"
#include "obexserver/obexserversession.h"
#include "agent/agent.h"



class KBlueTray : public QObject
{
Q_OBJECT

public:
	 KBlueTray(const QString& path, QObject* parent = 0);
	 ~KBlueTray();

private:

	 QStringList modes;
	 QString toolTipInfo;
	 QSystemTrayIcon* tray;
//	 KSystemTrayIcon* tray;
	 QMenu* mainmenu;

	 QAction* quitAction;
	 QAction* aboutAction;
	 QAction* adapterAction;
	 QAction* serverAction;
	 QAction* wizardAction;
	 QAction* sendToAction;
	 QAction* lockConfigureAction;
	 QAction* lockEnableAction;
	 QMenu* kbluelockMenu;
	 QMenu* settingsMenu;
	 
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


public slots:
	 void slotQuitApp(bool);
	 void showAboutInfo();
	 void showAdapterInterface();
	 void showWizard();
	 void showManager();
	 void updateTooltip();

	 void onlineMode();
	 void offlineMode();

	 void slotPropertyChanged(const QString&, const QVariant&);
	 void adapterAdded(const QString&);
	 void adapterRemoved(const QString&);
	 void defaultAdapterChanged(const QString&);

	 void enableLock();
	 void enableBlueLock(const QString&);
	 void slotValidateLockDev(const QString&);
	 void slotConfigureKBlueLock();
	 void disableBlueLock();
	 void slotBlueSelectorCancelled();
	 void slotTargetDeviceFound(const QString&, const QMap< QString,QVariant > &);
	 void slotTargetDeviceBack(const QString&, const QMap< QString,QVariant > &);
	 void slotUnlockScreenChanged(bool);

	 void obexSessionReady();
	 void sendFile();
	 void slotSendFile(const QString&);
	 void slotFileSendFinished();
	 void slotTransferProgress(qulonglong);
	 void slotFileSendCanceled();
	 void slotFileSendStarted(const QString& ,const QString& ,qulonglong );

	 void enableServer();
	 void slotServerReady();
	 void slotServerErrorOccured(const QString&, const QString&);
	 void slotServerStarted();
	 void slotServerStopped();
	 void slotServerClosed();
	 void slotServerSessionCreated(const QString&);
	 void slotServerSessionRemoved(const QString&);
	 void slotServerSessionTransferStarted(const QString&, const QString&, qulonglong);
	 void slotFileReceiveCancelled();
	 void slotServerSessionTransferCompleted();
	 void slotServerTransferDisconnected();

};


#endif
