//
// Copyright (C) 2011 David Eckhoff <eckhoff@cs.fau.de>
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

#ifndef LDMAPP_H_
#define LDMAPP_H_

#include "veins/modules/application/ieee80211p/BaseWaveApplLayer.h"
#include <map>
#include "veins/base/modules/BaseApplLayer.h"
#include "veins/modules/utility/Consts80211p.h"
#include "veins/modules/messages/WaveShortMessage_m.h"
#include "veins/base/utils/Coord.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/modules/mac/ieee80211p/WaveAppToMac1609_4Interface.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include <math.h>
#include <stdlib.h>
#include <coutvector.h>
#include "Opinions.h"


using Veins::TraCIMobility;
using Veins::TraCICommandInterface;
using Veins::AnnotationManager;

//implementation LDMApp.cc
int AttackersNumCntr = 0;

/** @brief description of a single local dynamic map entry.
 * @author Rens van der Heijden
 */
class LDMEntry {
  private:
    friend std::ostream& operator<<(std::ostream&, const LDMEntry&);

    simtime_t time;  //timestamp for the most recent update
    Coord pos; //most recent position
    int speed; //most recent speed value
    Opinions ART; //ART's (belief, disbelief, uncertainty)
    Opinions eART; //eART's (belief, disbelief, uncertainty)
    Opinions ExchOfLDM; //Proactive Exchange (belief, disbelief, uncertainty)
    Opinions Merged; //Merged opinion's (belief, disbelief, uncertainty)
    //  std::map<int,LDMEntry> LDMofMyNeighbor;
    int IDsForThisNeighbor [500];
    // mervat
    //   double ARTbeliefOpinion;
    //   double ARTdisbeliefOpinion;
    //   double ARTuncertaintyOpinion;
    // more data elements can be put here.

  public:
    LDMEntry(){
    }

    static LDMEntry LDMEntryNull;

    LDMEntry(const Coord p, const int v, const simtime_t t, const Opinions a, const Opinions eART, const Opinions exch, const Opinions M, const int IDs[500] ){ //, const double ARTbeliefop, const double ARTdisbeliefop, const double ARTuncertaintyop){
      this->pos=p;
      this->speed=v;
      this->time=t;
      this-> ART=a;
      this-> eART=eART;
      this-> ExchOfLDM=exch;
      this-> Merged=M;
      for(int i=0;i<500;i++){
        this-> IDsForThisNeighbor[i]=IDs[i];
      }
    }

    static bool IsNull(const LDMEntry& test){
      return LDMEntryNull == test;
    }

    LDMEntry& operator=(const LDMEntry& other){
      pos=other.pos;
      speed=other.speed;
      time=other.time;
      ART=other.ART;
      eART=other.eART;
      ExchOfLDM=other.ExchOfLDM;
      Merged=other.Merged;
      for(int i=0;i<500;i++){
        IDsForThisNeighbor[i]=other.IDsForThisNeighbor[i];
      }
      return *this;
    }

    LDMEntry(const LDMEntry& other){
      this->pos=other.getPosition();
      this->speed=other.getSpeed();
      this->time=other.getTime();
      this->ART=other.getART();
      this->eART=other.geteART();
      this->ExchOfLDM=other.getExchOfLDM();
      this->Merged=other.getMerged();
      for(int i=0;i<500;i++){
        this->IDsForThisNeighbor[i]=other.getIDsForThisNeighbor(i);
      }
    }

    /** Update this entry by replacing its contents with the those in the argument. */
    void update(LDMEntry other){
      this->pos=other.getPosition();
      this->speed=other.getSpeed();
      this->time=other.getTime();
      this->ART=other.getART();
      this->eART=other.geteART();
      this->ExchOfLDM=other.getExchOfLDM();
      this->Merged=other.getMerged();
      for(int i=0;i<500;i++){
        this->IDsForThisNeighbor[i]=other.getIDsForThisNeighbor(i);
      }
    }

    simtime_t getTime()const{return time;}
    Coord getPosition()const{return pos;}
    int getSpeed()const{return speed;}
    Opinions getART()const{return ART;}
    Opinions geteART()const{return eART;}
    Opinions getExchOfLDM()const{return ExchOfLDM;}
    Opinions getMerged()const{return Merged;}
    int getIDsForThisNeighbor(int k)const{return IDsForThisNeighbor[k];}

    /** Comparison for equality. */
    bool const operator==(const LDMEntry& other){
      return this->pos         == other.getPosition()
        && this->speed   == other.getSpeed()
        && this->time    == other.getTime();
    }
};

/** @brief Local Dynamic Map that uses BaseWaveApplLayer
 *
 * @author Rens van der Heijden
 *
 * @ingroup ldm
 *
 * @see TraCIDemo11p
 * @see BaseWaveApplLayer
 * @see Mac1609_4
 * @see PhyLayer80211p
 * @see Decider80211p
 */
class LDMApp : public BaseWaveApplLayer {
  private:
    simsignal_t distanceSignal;
    simsignal_t distanceAttackerSignal;
    simsignal_t attackerBeaconRcvSignal;
    simsignal_t benignBeaconRcvSignal;

    simsignal_t ARTTruePositivesSignal;
    simsignal_t ARTTrueNegativesSignal;
    simsignal_t ARTFalsePositivesSignal;
    simsignal_t ARTFalseNegativesSignal;

    simsignal_t eARTTruePositivesSignal;
    simsignal_t eARTTrueNegativesSignal;
    simsignal_t eARTFalsePositivesSignal;
    simsignal_t eARTFalseNegativesSignal;

    simsignal_t ExchangeTruePositivesSignal;
    simsignal_t ExchangeTrueNegativesSignal;
    simsignal_t ExchangeFalsePositivesSignal;
    simsignal_t ExchangeFalseNegativesSignal;

    simsignal_t MergedTruePositivesSignal;
    simsignal_t MergedTrueNegativesSignal;
    simsignal_t MergedFalsePositivesSignal;
    simsignal_t MergedFalseNegativesSignal;
  public:
    ~LDMApp();
    virtual void initialize(int stage);
    virtual void finish();

    Coord ModificationVector;
    Opinions ART;
    Opinions eART;
    Opinions ExchOfLDM;
    Opinions Merged;
    int counter1;
    int counter2;
    int counter3;
    int counter4;
    int counter5;
    int counterTestAnalysis =0;
    bool ThereIsTwoHopNeighbors;
    int MaxDis;
    int MinDis;
    bool SenderIsBenign;
    Coord NeighborPosition;
    int SenderEntryID;
    int DenominatorCounter=0;
    simtime_t StartTime;

    bool IamAnAttacker=false;

  protected:
    TraCIMobility* mobility;
    TraCICommandInterface* traci;
    TraCICommandInterface::Vehicle* traciVehicle;
    AnnotationManager* annotations;
    simtime_t lastDroveAt;
    bool sentMessage;
    bool isParking;
    //maps addresses to their respective LDM entries.
    std::map<int,LDMEntry> ldm;

  protected:
    //storage method -- override to add tests etc.
    virtual void storeInLDM(const int sender, const LDMEntry& data);
    //retrieval method
    virtual const LDMEntry fetchFromLDM(const int sender) const;
    //information about self: ID, position, speed, direction
    virtual const int getMyID() const;
    virtual const Coord getMyPosition() const;
    virtual const double getMySpeed() const;

    //reception method for beacons -- override when message type is different
    virtual void onBeacon(WaveShortMessage * wsm) override;
    //overrides BaseWaveApplLayer::prepareWSM -- not yet implemented.
    //virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channel, int priority, int rcvId, int serial=0) override;
    //overrides BaseWaveApplLayer::handlePositionUpdate
    virtual void handlePositionUpdate(cObject* obj) override;
    //overrides BaseWaveApplLayer::handleLowerMsg
    virtual void handleLowerMsg(cMessage* msg) override;
    //overrides BaseWaveApplLayer::handleSelfMsg
    virtual void handleSelfMsg(cMessage* msg) override;
    //
    virtual void onData(WaveShortMessage* wsm) override;

    // from TraCIDemo11p:
    //wrapper that sends a message of type "data":
    void sendMessage(std::string blockedRoadId);
    virtual void sendWSM(WaveShortMessage* wsm);
    double calcLength(double x0, double y0, double x1, double y1); //override;
};

inline std::ostream& operator<<(std::ostream &strm, const LDMEntry& ldmapp) {
  return strm << "LDMEntry(" << ldmapp.pos << ")";
}

#endif /* LDMAPP_H_ */
