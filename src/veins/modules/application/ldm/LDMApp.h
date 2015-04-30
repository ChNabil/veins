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

#ifndef TRACE
#define TRACE ev << "TRACING: " << getMyID() << ";" <<  getMyPosition() << ";" << simTime() << ";" << getMetaData()
#include<mutex>
std::mutex mutex; //mutex for logging
#endif



using Veins::TraCIMobility;
using Veins::TraCICommandInterface;
using Veins::AnnotationManager;

//implementation LDMApp.cc

/**
 * @brief description of a single local dynamic map entry.
 * @author Rens van der Heijden
 */
class LDMEntry {
        simtime_t time;  //timestamp for the most recent update
        Coord pos; //most recent position
        int speed; //most recent speed value
        // more data elements can be put here.

        public:
        LDMEntry(const Coord p, const int v, const simtime_t t){
                this->pos=p;
                this->speed=v;
                this->time=t;
        }
        
        LDMEntry& operator=(const LDMEntry& other){
                pos=other.pos;
                speed=other.speed;
                time=other.time;
                return *this;
        }
        
        LDMEntry(const LDMEntry& other){
                this->pos=other.getPosition();
                this->speed=other.getSpeed();
                this->time=other.getTime();
        }
        
        simtime_t getTime()const{return time;}
        Coord getPosition()const{return pos;}
        int getSpeed()const{return speed;}
};

/**
 * @brief LDM that uses BaseWaveApplLayer
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

	public:
		~LDMApp();
		virtual void initialize(int stage);
		virtual void finish();

	protected:
		TraCIMobility* mobility;
		TraCICommandInterface* traci;
		TraCICommandInterface::Vehicle* traciVehicle;
		AnnotationManager* annotations;
		simtime_t lastDroveAt;
		bool sentMessage;
		bool isParking;
                //see LDMApp.ned
		bool createTrace;
                std::string traceFile;
                //maps addresses to their respective LDM entries.
                std::map<int,LDMEntry> ldm;


	protected:
                //storage method -- override to add tests etc.
		virtual void storeInLDM(const int& sender, const LDMEntry& data);
                //retrieval method
                virtual const LDMEntry fetchFromLDM(const int sender) const;
                //information about self: ID, position, speed, direction
                virtual const int getMyID() const;
                virtual const Coord getMyPosition() const;
                virtual const double getMySpeed() const;
                virtual const double getAngle() const;
                //getMetaData is meta-data included with every output
                virtual const std::string getMetaData() const;
                //create trace output (only created when createTrace is set)
                virtual const void traceStep() const;

                //reception method for beacons -- override when message type is different
		virtual void onBeacon(WaveShortMessage * wsm) override;
                //overrides BaseWaveApplLayer::prepareWSM -- not yet implemented.
                //virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channle, int priority, int rcvId, int serial=0) override;
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
};

#endif /* LDMAPP_H_ */
