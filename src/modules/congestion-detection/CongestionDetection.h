// Copyright (C) 2014 Julian Schregle <julian.schregle@uni-ulm.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef CONGESTIONDETECTION_H_
#define CONGESTIONDETECTION_H_


#include "modules/application/ieee80211p/BaseWaveApplLayer.h"
#include "modules/mobility/traci/TraCIMobility.h"



using Veins::TraCIMobility;
using Veins::AnnotationManager;

class CongestionDetection : public BaseWaveApplLayer{

public:
	class beaconDataElement{
	private:
		int id; //id of node
		int d; //direction
		double locx; //location x
		double locy; //location y
		double velocity;
	public:
		void initialize();
		void setId(int index);
		int getID();
		void setDirection(int dir);
		int getDir();
		void setLocX(double x);
		double getLocX();
		void setLocY(double y);
		double getLocY();
		void setVelocity(double v);
		double getVelocity();
	};
	CongestionDetection();
	virtual ~CongestionDetection();
	//void changeTraCIData(double currentSpeed, double posx, double posy, double nangele);

	protected:
		int CarID; //ID of the car, witch should use Congestion Detection
		std::string externalID; //sumo name
		std::string compareWithTV; //test vehicle
		int L_T; // Location Type identifies Road Type; default(Autobahn);
		int I_T; // Index time in relationship with Location Type
		int I_L; // Index of Lanes; default(1) cause of Location/Road Type
		double I_TRUCK; //Index of Trucks; Truck share of the total traffic default(0) in percent --> 30 are 30%
		double V_C; // current velocity of the Vehicle
		double V_T; // velocity threshold, based on RoadType default(30);
		double V_ADD; // added velocity after current velocity below threshold
		int I_V; // index of Velocity to calculate average velocity
		int I_LDM; // index of Local Dynamic Map; contains the number of Vehicles in LDM
		bool B_CD; //boolean Congestion Detected; default(false) no congestion detected; shows if algorithm has congestion Detected
		int I_M; //contains number of entries in Warning message
		double L_C_X; // current Location x coorinate
		double L_C_Y; //current Location y coordinate
		double angle;
		int F_TRUCK;
		int F_V;
		bool sB;

		bool started;
		cMessage *ntime;
		cMessage *btime;

		int ctr;

		TraCIMobility* traci;
		AnnotationManager* annotations;

		std::vector<std::string> M_T; //Warning message Table
		std::vector<beaconDataElement> LDM; //Local Dynamic Map Table

	protected:

	     virtual void initialize(int stage);
		 virtual void handleSelfMsg(cMessage *msg);						//implemening not finished
		 double calcVelocityAverage();									//implemented


		 void congestionDetected();										//implemented
		 int calcDirection();											//implemented
		// void handleWarningMsg(cMessage *msg);
		// void sendWarningMsg();

		 double calcCongestionLength();									//implemented
		 double calcLength(double x0, double y0, double x1, double y1);	//implemented
		 Coord getCoordFromString(std::string s);						//implemented

		////////////////////WSM Messageing ////////////
		virtual void sendWSM(WaveShortMessage* wsm);					//implemented
		void sendMessage(WaveShortMessage* wsm);						//implemented
		virtual void onBeacon(WaveShortMessage* wsm);
		virtual void handleIncommingBeacons(std::string beacon);
		virtual void onData(WaveShortMessage* wsm);						//implemented

		std::vector<std::string> parseWarning(WaveShortMessage  *msg);	//implemented
		 WaveShortMessage* buildWarning(std::vector<std::string> list);	//implemented
		 WaveShortMessage* buildWarning(std::string s);					//implemented
		 void paintList(std::vector<std::string> ls);					//implemented
		 std::string buildString(std::vector<std::string> list);		//implemented

		 bool searchIdinLDM(unsigned int id);
		 int searchIdinLDMPos(int id);
		 beaconDataElement parseStringToBDE(std::string s);
};

#endif /* CONGESTIONDETECTION_H_ */
