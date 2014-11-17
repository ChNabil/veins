//
// Copyright (C) 2012 David Eckhoff <eckhoff@cs.fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "LDMApp.h"

void LDMApp::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
}


void LDMApp::handlePositionUpdate(cObject * obj) {
        BaseWaveApplLayer::handlePositionUpdate(obj);
        //storeInLDM(SELF, updatedEntry);
}

void LDMApp::handleLowerMsg(cMessage * msg) {
	WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg);
	ASSERT(wsm);

        //this module only understands beacons
	if (std::string(wsm->getName()) == "beacon") {
		onBeacon(wsm);
	}
	else {
		DBG << "unknown message (" << wsm->getName() << ")  received\n";
	}
	delete(msg);
}

void LDMApp::handleSelfMsg(cMessage * msg) {
	switch (msg->getKind()) {
		case SEND_BEACON_EVT: {
                        WaveShortMessage* wsm = prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
                        //TODO add additional payload to WSM.
			sendWSM(wsm);
			scheduleAt(simTime() + par("beaconInterval").doubleValue(), BaseWaveApplLayer::sendBeaconEvt);
			break;
		}
		default: {
			if (msg)
				DBG << "APP: Error: Got Self Message of unknown kind! Name: " << msg->getName() << endl;
			break;
		}
	}
}

void LDMApp::onBeacon(WaveShortMessage * wsm){
        LDMEntry* data = new LDMEntry();
        //data->pos=;
        //data->speed=;
        //data->time=currentTime;
        storeInLDM(wsm->getSenderAddress(), *data);
}

const LDMEntry& LDMApp::fetchFromLDM(const int sender) const{
        return ldm.at(sender);
}

void LDMApp::storeInLDM(const int sender, LDMEntry& data){
        ldm[sender]=data;
}

void LDMApp::finish() {
	if (sendBeaconEvt->isScheduled()) {
		cancelAndDelete(sendBeaconEvt);
	}
	else {
		delete sendBeaconEvt;
	}

	findHost()->unsubscribe(mobilityStateChangedSignal, this);
        //clean up the LDM
        for(  std::map<int,LDMEntry>::iterator entry=ldm.begin(); entry!=ldm.end(); ++entry)
        {
                //destroy the LDMEntry object
                delete &(entry->second);
        }
        //destroy the map itself
        delete &(ldm);
}

LDMApp::~LDMApp() {

}
