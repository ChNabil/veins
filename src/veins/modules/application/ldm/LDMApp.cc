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
#include "Opinions.h"

#include <cmath>        // std::abs

#ifndef myEV
#define myEV ev
#endif

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;

Define_Module(LDMApp);

/* 
   *********************** LDMApp methods ****************************
*/

LDMEntry LDMEntry::LDMEntryNull = LDMEntry();

const LDMEntry LDMApp::fetchFromLDM(const int sender) const {
  DBG << "Current LDM contents:"  << std::endl;
  for (auto& kv : ldm) {
    DBG << kv.first << " has position " << kv.second << std::endl;
  }
  std::map<int,LDMEntry>::const_iterator it = ldm.find(sender);
  if(it!=ldm.end())
    return it->second;
  else
    return LDMEntry::LDMEntryNull;
}

void LDMApp::storeInLDM(const int sender, const LDMEntry& data) {
  LDMEntry r = fetchFromLDM(sender);
  if(LDMEntry::IsNull(r)){
    DBG << "new neighbor"<< this->myId << ": " << sender << " with position: " << data<< std::endl;
    ldm.insert(std::pair<int,LDMEntry>(sender, data));
  }else{
    DBG <<  this->myId << " already have data for " << sender << ": " << r << std::endl;
    //r.update(data);
    ldm[sender]=data;
    DBG << this->myId << " updated Entry: " << r << std::endl;
  }
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

/* 
   **************** Overridden methods *******************
*/

void LDMApp::initialize(int stage) {
  if(!par("validConfiguration").boolValue())
    throw cRuntimeError("Loading LDMApp without config!");
  BaseWaveApplLayer::initialize(stage);
  distanceSignal         = registerSignal("distance");
  distanceAttackerSignal = registerSignal("distanceAttacker");
  attackerBeaconRcvSignal= registerSignal("attackerBeacons");
  benignBeaconRcvSignal  = registerSignal("benignBeacons");

  ARTTruePositivesSignal = registerSignal("ARTTruePositives");
  ARTTrueNegativesSignal = registerSignal("ARTTrueNegatives");
  ARTFalsePositivesSignal = registerSignal("ARTFalsePositives");
  ARTFalseNegativesSignal = registerSignal("ARTFalseNegatives");

  eARTTruePositivesSignal = registerSignal("eARTTruePositives");
  eARTTrueNegativesSignal = registerSignal("eARTTrueNegatives");
  eARTFalsePositivesSignal = registerSignal("eARTFalsePositives");
  eARTFalseNegativesSignal = registerSignal("eARTFalseNegatives");

  ExchangeTruePositivesSignal = registerSignal("ExchangeTruePositives");
  ExchangeTrueNegativesSignal = registerSignal("ExchangeTrueNegatives");
  ExchangeFalsePositivesSignal = registerSignal("ExchangeFalsePositives");
  ExchangeFalseNegativesSignal = registerSignal("ExchangeFalseNegatives");

  MergedTruePositivesSignal = registerSignal("MergedTruePositives");
  MergedTrueNegativesSignal = registerSignal("MergedTrueNegatives");
  MergedFalsePositivesSignal = registerSignal("MergedFalsePositives");
  MergedFalseNegativesSignal = registerSignal("MergedFalseNegatives");

  if (stage == 0) {
    mobility = TraCIMobilityAccess().get(getParentModule());
    traci = mobility->getCommandInterface();
    traciVehicle = mobility->getVehicleCommandInterface();
    annotations = AnnotationManagerAccess().getIfExists();
    ASSERT(annotations);

    if (getMyID()==0){
      AttackersNumCntr = 0;
    }

    double probability = par("maliciousProbability").doubleValue();
    if( (probability < 0 && AttackersNumCntr == 0) // < 0 means *exactly one* attacker.
        || (probability > 0)){
      IamAnAttacker = (dblrand() <= std::abs(probability));
      if(IamAnAttacker){
        AttackersNumCntr++;
      }
    }

    StartTime=simTime();
    sentMessage = false;
    lastDroveAt = simTime();

    //TODO
    scheduleAt(simTime() + par("beaconInterval").doubleValue() + (0.0005 * getMyID()), BaseWaveApplLayer::sendBeaconEvt);
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
      counter5=0;
      
      WaveShortMessage* wsm = prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
      
      for (int i=0; i<500; i++){            // Added by Ala'a
        wsm->setIntVector(i,-1);
      }

      for(auto SenderEntry=ldm.begin(); SenderEntry!=ldm.end(); ++SenderEntry){ // Added by Ala'a
        SenderEntryID=SenderEntry->first;
        // ListOfIDs [counter5] = SenderEntryID;
        wsm->setIntVector(counter5,SenderEntryID);
        counter5++;
      }

      if(IamAnAttacker){
        wsm->setSenderIsAttackerInWSM(true);
        //fetch manipulation parameters from the configuration.
        //this can return the same value or a value from some distribution.
        Coord senderPosModified;
        if(par("attackerType").doubleValue() == 0.0){
            //random false position
            senderPosModified = Coord(par("attackerParameter"), par("attackerParameter"));
        }else if (par("attackerType").doubleValue() == 1.0){
            //modify the transmitted position dynamically
            ModificationVector.x = par("attackerParameter");
            ModificationVector.y = par("attackerParameter");
            senderPosModified = wsm->getSenderPos() + ModificationVector;
        }

        wsm->setSenderPos(senderPosModified);
      }else{
        wsm->setSenderIsAttackerInWSM(false);
      }
      
      sendWSM(wsm);
      //TODO improve this by using a randomized start time instead
      scheduleAt(simTime() + par("beaconInterval").doubleValue() + (0.0005 * getMyID()), BaseWaveApplLayer::sendBeaconEvt);
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
  // stopped for at least 10s?
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
  int senderID = wsm->getSenderAddress();
  myEV << "Encountered a new beacon from " << senderID << " with intended recipient " << wsm->getRecipientAddress() << " containing the following position: " << wsm->getSenderPos() << " and timestamped at simulation time " << wsm->getTimestamp() << std::endl;
  double distance = calcLength(getMyPosition().x,getMyPosition().y,wsm->getSenderPos().x,wsm->getSenderPos().y);

  //distanceSignal measures the average reception distance to legitimate nodes.
  if(wsm->getSenderIsAttackerInWSM()) {
    emit(distanceSignal, calcLength(getMyPosition().x,getMyPosition().y,wsm->getSenderPos().x,wsm->getSenderPos().y));
  } else {
    emit(distanceAttackerSignal, calcLength(getMyPosition().x,getMyPosition().y,wsm->getSenderPos().x,wsm->getSenderPos().y));
  }
  //attackers do not contribute measurements, and we have a warm-up of 5 seconds where we only store.
  if(!IamAnAttacker && simTime()-StartTime > 5.0){
    DenominatorCounter++;
    if(wsm->getSenderIsAttackerInWSM()){
      emit(attackerBeaconRcvSignal, 1);
    }else{
      emit(benignBeaconRcvSignal, 1);
    }

    if(distance>par("ART_threshold").doubleValue()){
      //detection happens
      if(wsm->getSenderIsAttackerInWSM()){
        emit(ARTTruePositivesSignal, 1);
      }else{
        emit(ARTFalsePositivesSignal, 1);
      }
    }else{
      //detection does not happen
      if(wsm->getSenderIsAttackerInWSM()){
        emit(ARTFalseNegativesSignal, 1);
      }else{
        emit(ARTTrueNegativesSignal, 1);
      }
    }

    ///////////////////Enhancement for the paper///////////////////////////////
    double range=par("eART_threshold").doubleValue();
    double variance=par("eART_variance").doubleValue();
    double maxUncertainty=par("eART_max_uncertainty").doubleValue();
    //Gaussian distribution of uncertainty
    double uncertainty = maxUncertainty * std::exp(-std::pow(std::abs((distance - range)), 2) / (2 * variance * variance));
    double certainty = 1 - uncertainty;
    double disbeliefFactor = (1.0/(2*range) * distance);

    double disbelief, belief;
    if(distance > range){
        disbelief =  certainty;
        belief = 0;
    } else {
      disbelief = 0;
      belief = certainty;
    }

    eART.setbelief(belief);
    eART.setdisbelief(disbelief);
    eART.setuncertainty(uncertainty);
    if(!eART.isConsistent()){
      std::cout << "Warning, opinion eART is inconsistent " << eART.getbelief() << " " << eART.getdisbelief() << " " << eART.getuncertainty() << std::endl;
    }
    ///////////////////Enhancement for the paper///////////////////////////////

    double estimate = eART.getbelief() + eART.getuncertainty()*0.5; //assumes base rate of 0.5
    if(estimate<par("detectionThreshold").doubleValue()){
      //detection happens
      if(wsm->getSenderIsAttackerInWSM()){
        emit(eARTTruePositivesSignal, 1);
      }else{
        emit(eARTFalsePositivesSignal, 1);
      }
    } else {
      //detection does not happen
      if(wsm->getSenderIsAttackerInWSM()){
        emit(eARTFalseNegativesSignal, 1);
      }else{
        emit(eARTTrueNegativesSignal, 1);
      }
    }

    ////////////////////// end of the edit for ART Sensor /////////////////////////////////////
    //  this->ARTbeliefOpinion = LDMApp::Opinions::getbelief();

    /////////////////////////////// proactive exchange of neighbors LDM /////////////////////////////////////////////
    Coord SenderClaimedPosition = wsm->getSenderPos();
    bool SenderIsBenign = false;
    int SenderIsBenignCounter=0;
    int SenderIsNotBenignCounter=0;
    bool SenderAppearsInLDMofNeighborWhileItShouldNotAppear=0;

    for(auto entry=ldm.begin(); entry!=ldm.end(); ++entry){
      SenderIsBenign=false;
      SenderAppearsInLDMofNeighborWhileItShouldNotAppear=false;
      
      if (senderID != entry->first){
        LDMEntry LDMelements=entry->second;
        NeighborPosition=LDMelements.getPosition();
        if(calcLength(SenderClaimedPosition.x, SenderClaimedPosition.y, NeighborPosition.x, NeighborPosition.y)<=par("Exchange_threshold").doubleValue()){
          // sender should appear in the neighbor LDM

          // define a time window such that a LDM entry that is older than 5 second will be discarded, and the test will be discarded  if the time difference between the received beacon and the LDM entry is more than 3 second
          if(((simTime() - LDMelements.getTime()) <= 5) && ((wsm->getTimestamp() - LDMelements.getTime()) <= 5)){
            for (int i=0; i<500; i++){
              if(senderID==LDMelements.getIDsForThisNeighbor(i)){
                SenderIsBenign=true;
                SenderIsBenignCounter++;
                break;
              }
            }
            if (!SenderIsBenign){
              SenderIsNotBenignCounter++;
            }
          }
        }else{
          // sender should NOT appear in the neighbor LDM
          if(((simTime() - LDMelements.getTime()) <= 5) && ((wsm->getTimestamp() - LDMelements.getTime()) <= 5)){

            for (int i=0; i<500; i++){
              if(senderID==LDMelements.getIDsForThisNeighbor(i)){
                SenderIsNotBenignCounter++;
                SenderAppearsInLDMofNeighborWhileItShouldNotAppear=true;
                break;
              }
            }

            if (!SenderAppearsInLDMofNeighborWhileItShouldNotAppear){
              SenderIsBenignCounter++;
            }
          }

        }
      } //senderID is entry->first
    } //end of the outer loop over all entries
 
    int numNeighbor = SenderIsBenignCounter + SenderIsNotBenignCounter;
    if(numNeighbor > 0){
      double uncertainty = std::exp(-par("exchangeUncertaintyFactor").doubleValue() * numNeighbor);
      ExchOfLDM.setbelief(((double)SenderIsBenignCounter)/((double)numNeighbor)*(1-uncertainty));
      ExchOfLDM.setdisbelief(((double)SenderIsNotBenignCounter)/((double)numNeighbor)*(1-uncertainty));
      ExchOfLDM.setuncertainty(uncertainty);
      if(!ExchOfLDM.isConsistent()){
        std::cout << "Warning, opinion ExchOfLDM is inconsistent " << ExchOfLDM.getbelief() << " " << ExchOfLDM.getdisbelief() << " " << ExchOfLDM.getuncertainty() << std::endl;
      }

      estimate = ExchOfLDM.getbelief()+ExchOfLDM.getuncertainty()*0.5;
      if(estimate<par("detectionThreshold").doubleValue()){
        //detection happens
        if(wsm->getSenderIsAttackerInWSM()){
          emit(ExchangeTruePositivesSignal, 1);
        }else{
          emit(ExchangeFalsePositivesSignal, 1);
        }
      } else {
        //detection does not happen
        if(wsm->getSenderIsAttackerInWSM()){
          emit(ExchangeFalseNegativesSignal, 1);
        }else{
          emit(ExchangeTrueNegativesSignal, 1);
        }
      }

      //////////////////////////// end of proactive exchange of neighbors LDM /////////////////////////////////////////

      //////////////////////////// edit for consensus operator /////////////////////////////////////////
      double k;
      double Temp_b;
      double Temp_d;
      double Temp_u;

      k = eART.getuncertainty() + ExchOfLDM.getuncertainty() - eART.getuncertainty() * ExchOfLDM.getuncertainty();
      if(k==0) {
        Temp_b = 0.0;
        Temp_d = 0.0;
        Temp_u = 1.0;
      } else {
        Temp_b = ((eART.getbelief() * ExchOfLDM.getuncertainty() + ExchOfLDM.getbelief() * eART.getuncertainty()) / k);
        Temp_d = ((eART.getdisbelief() * ExchOfLDM.getuncertainty() + ExchOfLDM.getdisbelief() * eART.getuncertainty()) / k);
        Temp_u = (eART.getuncertainty() * ExchOfLDM.getuncertainty() / k);
      }

      Merged.setbelief(Temp_b);
      Merged.setdisbelief(Temp_d);
      Merged.setuncertainty(Temp_u);
      if(!Merged.isConsistent()){
        std::cout << "Warning, opinion Merged is inconsistent " << Merged.getbelief() << " " << Merged.getdisbelief() << " " << Merged.getuncertainty() << std::endl;
      }

      estimate = Merged.getbelief() + Merged.getuncertainty() * 0.5;
      if(estimate<par("detectionThreshold").doubleValue()) {
        //detection happens
        if(wsm->getSenderIsAttackerInWSM()) {
          emit(MergedTruePositivesSignal, 1);
        } else {
          emit(MergedFalsePositivesSignal, 1);
        }
      } else {
        //detection does not happen
        if (wsm->getSenderIsAttackerInWSM()) {
          emit(MergedFalseNegativesSignal, 1);
        } else {
          emit(MergedTrueNegativesSignal, 1);
        }
      }
      //////////////////////////// end of the edit for consensus operator /////////////////////////////////////////
    }else{
      //zero items, so do nothing.
    }
  }

  int IDsToBeStored[500];
  for (int i=0; i<500; i++){
    IDsToBeStored[i] = wsm->getIntVector(i);
  }
  Coord tmp = wsm->getSenderPos();
  LDMEntry data{tmp, 0, simTime(), ART, eART, ExchOfLDM, Merged, IDsToBeStored};
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
  recordScalar("IamAnAttacker", IamAnAttacker);
  recordScalar("AttackersNumCntr", AttackersNumCntr);
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
  WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel, dataPriority, -1,2);
  wsm->setWsmData(blockedRoadId.c_str());
  sendWSM(wsm);
}

void LDMApp::sendWSM(WaveShortMessage* wsm) {
  sendDelayedDown(wsm,individualOffset);
}

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
