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

void LDMApp::storeInLDM(const int sender, const LDMEntry& data) {
        DBG << "Update for " << sender << std::endl;
        ldm.insert(std::pair<int,LDMEntry>(sender, data));
}

const int LDMApp::getMyID() const {
        return getParentModule()->getIndex(); //Car.ned's index.
}

const Coord LDMApp::getMyPosition() const {
        return mobility->getPositionAt(simTime());
}

const double LDMApp::getMySpeed() const {
        return mobility->getSpeed();
}

const double LDMApp::getAngle() const {
        return mobility->getAngleRad();
}

const std::string LDMApp::getMetaData() const {
        return "";
}

const void LDMApp::traceStep() const {
        if(createTrace){
                TRACE << std::endl;
        }
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

	    	ModificationVector.x = 0.0;
	        ModificationVector.y = 0.0;
	        ModificationVector.z = 0.0;

	//	WaveShortMessage* wsm = prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
		  //  sendWSM(wsm);
		//handleSelfMsg(wsm);
	   // scheduleAt(simTime() + par("beaconInterval").doubleValue(), BaseWaveApplLayer::sendBeaconEvt);
	    scheduleAt(simTime(), BaseWaveApplLayer::sendBeaconEvt);

	}

//	scheduleAt(simTime() + par("beaconInterval").doubleValue(), BaseWaveApplLayer::sendBeaconEvt);
//	scheduleAt(simTime() , BaseWaveApplLayer::sendBeaconEvt);



}

void LDMApp::handleLowerMsg(cMessage* msg) {
    ev<<" entered handlelowermsg ";
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
    ev <<" entered handleselfmsg ";
	switch (msg->getKind()) {
		case SEND_BEACON_EVT: {

                        WaveShortMessage* wsm = prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
                        wsm->setSenderAngle(mobility->getAngleRad()); // Added by Ala'a
                        ev<<" heading angle is " <<  wsm->getSenderAngle() << "  "; // added for debugging
//////////////// how to modify the position to implement the attack:
// if im node number 20 (the attacker):
//virtual void setSenderPos(const Coord& senderPos);
//this code is taken from waveshortmessage_m.h, more explanation are found on Github->Rens notes
////////////////////////////////////////////////////////////////////
if (getMyID() == 8){
//void setSenderPos(const Coord& senderPos);
   // senderPos = wsm->getSenderPos();
senderPosModified = wsm->getSenderPos() + ModificationVector;
wsm->setSenderPos(senderPosModified); // get rid of void
}


                        ev << "Created a beacon on channel " << type_CCH << " -- " << wsm << std::endl;
                        ev << " position of the sender: " << wsm->getSenderPos().x << " , " << wsm->getSenderPos().y << std::endl; // added for debugging
                        //TODO add additional payload to WSM.
			sendWSM(wsm);
			scheduleAt(simTime() + par("beaconInterval").doubleValue() + (0.05 * getMyID()), BaseWaveApplLayer::sendBeaconEvt);
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
    ev<<" entered handlepositionupdate11 ";
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
        traceStep();
}

void LDMApp::onBeacon(WaveShortMessage* wsm) {
    ev<<" entered onbeacon ";
        ev << "Encountered a new beacon from " << wsm->getSenderAddress() << " with intended recipient " << wsm->getRecipientAddress() << " containing the following position: " << wsm->getSenderPos() << " and timestamped at simulation time " << wsm->getTimestamp() << std::endl;
        ev<<" heading angle of the sender is " <<  wsm->getSenderAngle() << " the receiver angle is: " << mobility->getAngleRad() << std::endl; // added for debugging
        senderAngle = wsm->getSenderAngle(); // added by Ala'a
        // LDMEntry data{wsm->getSenderPos(), wsm->getSenderSpeed(), simTime()};

        //////////////////////edit for ART sensor//////////////////

               // sender_pos_x = wsm->getSenderPos().x;
              //  sender_pos_y = wsm->getSenderPos().y;
                //Coord c_p = traci->getCurrentPosition();
              //  c_p_x = getMyPosition().x; //Rx X position
              //  c_p_y = getMyPosition().y; //Rx Y position
       // distance = 200;
         distance = calcLength (getMyPosition().x,getMyPosition().y,wsm->getSenderPos().x,wsm->getSenderPos().y);
                if(distance>ART_threshold){
                ART.setbelief(0.0);
                ART.setdisbelief(1.0);
                ART.setuncertainty(0.0);
                ev<<" Warning: ART Opinion is <0.0,1.0,0.0> ";
                ev <<" ART Opinion is: Belief " << ART.getbelief() << ", Disbelief:" << ART.getdisbelief()<<", Uncertainty "<< ART.getuncertainty() << " " << std::endl;

                }else{
                ART.setbelief(0.5);
                ART.setdisbelief(0);
                ART.setuncertainty(0.5);
                ev<<"ART Opinion is <0.5,0.0,0.5>" << std::endl;
                ev <<" ART Opinion is: Belief " << ART.getbelief() << ", Disbelief:" << ART.getdisbelief()<<", Uncertainty "<< ART.getuncertainty() << " " << std::endl;

                }
                ev<<" distance is "<< distance << std::endl;


            ///////////////////////////////////////////////////////////
             //  this->ARTbeliefOpinion = LDMApp::Opinions::getbelief();

///////////////////////// edit for Overlap Sensor ///////////////////////////////////////////
LF_sender.x = (wsm->getSenderPos().x) - (2/2);
LF_sender.y = (wsm->getSenderPos().y) + (4/2);
RF_sender.x = (wsm->getSenderPos().x) + (2/2);
RF_sender.y = (wsm->getSenderPos().y) + (4/2);
LR_sender.x = (wsm->getSenderPos().x) - (2/2);
LR_sender.y = (wsm->getSenderPos().y) - (4/2);
RR_sender.x = (wsm->getSenderPos().x) + (2/2);
RR_sender.y = (wsm->getSenderPos().y) - (4/2);
senderID = wsm->getSenderAddress();
P[0]=LR_sender;
P[1]=LF_sender;
P[2]=RF_sender;
P[3]=RR_sender;

for(auto entry=ldm.begin(); entry != ldm.end(); entry++){
int id = entry->first;
LDMEntry ldmentry = entry->second;
 //  ev << " entered for loop for Overlap id of entry: " << id << " ldm entry " << ldmentry << std::endl;
ev << " node to be tested by overlap detector: " << id << std::endl;
    if (id != senderID){      //if (entry != (wsm->getSenderAddress())){
		//data_to_be_checked= fetch (entry);
           // receiver_data = fetchFromLDM(entry);     // get the data for the node in ldm
             //   center_pos_rx = receiver_data.getPosition();
        center_pos_rx = ldmentry.getPosition();
        ev << " center position of the node to be tested: " << center_pos_rx.x << " , " << center_pos_rx.y << std::endl; // added for debugging
 		LF_receiver.x = (center_pos_rx.x) - (2/2);
		LF_receiver.y = (center_pos_rx.y) + (4/2);
		RF_receiver.x = (center_pos_rx.x) + (2/2);
		RF_receiver.y = (center_pos_rx.y) + (4/2);
		LR_receiver.x = (center_pos_rx.x) - (2/2);
		LR_receiver.y = (center_pos_rx.y) - (4/2);
		RR_receiver.x = (center_pos_rx.x) + (2/2);
		RR_receiver.y = (center_pos_rx.y) - (4/2);

		ev << " Vertices of this node is: " << LR_receiver.x << " " << LR_receiver.y << LF_receiver.x << " " << LF_receiver.y << std::endl; // added for debugging
//		T[0] = LR_receiver;     //the sim is terminating because of those lines!!
//		T[1] = LF_receiver;     //the sim is terminating because of those lines!!
//		T[2] = RF_receiver;     //the sim is terminating because of those lines!!
//		T[3] = RR_receiver;     //the sim is terminating because of those lines!!
//		T[4] = LR_receiver;     //the sim is terminating because of those lines!!
//for (int i=0; i<4; i++){      // for P test point from the sender
//	for (int j=0; j<4; j++){	  // for ldm nodes' vertices
//	X[j] = ((P[i].x - T[j].x)*(T[j+1].y - T[j].y)) - ((P[i].y - T[j].y)*(T[j+1].x - T[j].x));
			    }
//				if (((X[0] < 0.0) && (X[1] < 0.0) && (X[2] < 0.0) && (X[3] < 0.0)) || ((X[0] > 0.0) && (X[1] > 0.0) && (X[2] > 0.0) && (X[3] > 0.0))){
//				Overlap.setbelief(0.0);
//				Overlap.setdisbelief(1.0);
//				Overlap.setuncertainty(0.0);
//				break;
//				}else{
//				Overlap.setbelief(0.5);
//				Overlap.setdisbelief(0.0);
//				Overlap.setuncertainty(0.5);
//							    }
//}
//		}
}
// ev <<" Overlap Opinion is: Belief " << Overlap.getbelief() << ", Disbeliefe " << Overlap.getdisbelief()<<", Uncertainty "<<Overlap.getuncertainty() << " " << std::endl ;

///////////////////////////////// end of the edit for Overlap Sensor ////////////////////////////////////////////

        LDMEntry data{wsm->getSenderPos(), 0, simTime(), senderAngle ,ART, Overlap}; //, ART.getbelief(), ART.getdisbelief(), ART.getuncertainty()}; ////
        storeInLDM(wsm->getSenderAddress(), data);
}

void LDMApp::finish() {
    ev<<" entered finish ";
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
    ev<<" entered ondata ";
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
    ev<<" entered sendmessage ";
        sentMessage = true;

        t_channel channel = dataOnSch ? type_SCH : type_CCH;
        WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
        wsm->setWsmData(blockedRoadId.c_str());
        sendWSM(wsm);
}

void LDMApp::sendWSM(WaveShortMessage* wsm) {
    ev<<" entered sendwsm ";
	sendDelayedDown(wsm,individualOffset);
}

///////////////////class "Opinions" functions///////////////

void Opinions::setbelief(double bel) {
belief = bel;
}
double Opinions::getbelief() {
return belief;
}
void Opinions::setdisbelief(double disbel) {
disbelief = disbel;
}
double Opinions::getdisbelief() {
return disbelief;
}
void Opinions::setuncertainty(double uncert) {
uncertainty = uncert;
}
double Opinions::getuncertainty() {
return uncertainty;
}
//////////////////////////////////////////////////////////

//calculates the distance between two points
double LDMApp::calcLength(double x0, double y0, double x1, double y1) {
    int x;
    int y;
    double distance;
    if (x0 >= x1) {
        x = x0 - x1;
        y = y0 - y1;
    } else {
        x = x1 - x0;
        y = y1 - y0;
    }
    if (x * x + y * y >= 0)
        distance = sqrt(x * x + y * y);
    else
        distance = sqrt(-1 * (x * x + y * y));

    return distance;
}
/////////////////////////////////////////////////////////////
