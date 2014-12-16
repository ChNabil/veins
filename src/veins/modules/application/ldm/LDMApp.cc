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

// see also LDMApp.ned
#include "LDMApp.h"

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

Define_Module(LDMApp);

/* 
   *********************** LDMApp methods ****************************
*/

const LDMEntry LDMApp::fetchFromLDM(const int sender) const {
        return ldm.at(sender);
}

void LDMApp::storeInLDM(const int& sender, const LDMEntry& data) {
        EV << "Update for " << sender << std::endl;
        ldm.insert(std::pair<int,LDMEntry>(sender, data));
}


/* 
   **************** Overridden methods *******************
*/

void LDMApp::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
	if (stage == 0) {
		mobility = TraCIMobilityAccess().get(getParentModule());
		traci = mobility->getCommandInterface();
		traciVehicle = mobility->getVehicleCommandInterface();
		annotations = AnnotationManagerAccess().getIfExists();
		ASSERT(annotations);

		sentMessage = false;
		lastDroveAt = simTime();
	}
}

void LDMApp::handleLowerMsg(cMessage* msg) {
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

void LDMApp::handleSelfMsg(cMessage* msg) {
	switch (msg->getKind()) {
		case SEND_BEACON_EVT: {
                        WaveShortMessage* wsm = prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
                        ev << "Created a beacon on channel " << type_CCH << " -- " << wsm << std::endl;
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

void LDMApp::handlePositionUpdate(cObject* obj) {
        BaseWaveApplLayer::handlePositionUpdate(obj);
        //storeInLDM(SELF, updatedEntry);

        //code from the TraCIDemo11p:
	// stopped for for at least 10s?
	if (mobility->getSpeed() < 1) {
		if (simTime() - lastDroveAt >= 10) {
                        // visualize that the car is stopped
			findHost()->getDisplayString().updateWith("r=16,red");
		}
	}
	else {
		lastDroveAt = simTime();
	}
        //end of code from the TraCIDemo11p
}

void LDMApp::onBeacon(WaveShortMessage* wsm) {
        EV << "Encountered a new beacon from " << wsm->getSenderAddress() << " with intended recipient " << wsm->getRecipientAddress() << " containing the following position: " << wsm->getSenderPos() << " and timestamped at simulation time " << wsm->getTimestamp() << std::endl;
        LDMEntry data{wsm->getSenderPos(), wsm->getSenderSpeed(), simTime()};
        storeInLDM(wsm->getSenderAddress(), data);
}

void LDMApp::finish() {
	if (sendBeaconEvt->isScheduled()) {
		cancelAndDelete(sendBeaconEvt);
	}
	else {
		delete(sendBeaconEvt);
	}

	findHost()->unsubscribe(mobilityStateChangedSignal, this);
        //clean up the LDM
        for(auto entry=ldm.begin(); entry!=ldm.end(); ++entry)
        {
                //destroy the LDMEntry object
                //delete(entry->second);
        }
        //destroy the map itself
        //delete(&ldm);
}

void LDMApp::onData(WaveShortMessage* wsm) {
	//Code from TraCIDemo11p
	findHost()->getDisplayString().updateWith("r=16,green");
	annotations->scheduleErase(1, annotations->drawLine(wsm->getSenderPos(), mobility->getPositionAt(simTime()), "blue"));

	if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getWsmData(), 9999);
	if (!sentMessage) sendMessage(wsm->getWsmData());
	//end code from TraCIDemo11p

}

LDMApp::~LDMApp() {

}


//CODE BEYOND THIS LINE IS FROM TraCIDemo11p!!!!

void LDMApp::sendMessage(std::string blockedRoadId) {
        sentMessage = true;

        t_channel channel = dataOnSch ? type_SCH : type_CCH;
        WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);        wsm->setWsmData(blockedRoadId.c_str());
        sendWSM(wsm);
}

void LDMApp::sendWSM(WaveShortMessage* wsm) {
	sendDelayedDown(wsm,individualOffset);
}
