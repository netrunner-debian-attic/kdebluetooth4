#! /usr/bin/env bash
$EXTRACTRC *.ui agent/*.ui deviceselector/*.ui obexserver/*.ui obexsession/*.ui device-manager/*.ui inputwizard/*.ui >> rc.cpp
$XGETTEXT *.cpp agent/*.cpp deviceselector/*.cpp obexserver/*.cpp  obexsession/*.cpp device-manager/*.cpp inputwizard/*.cpp -o $podir/kbluetooth.pot
rm -f rc.cpp
