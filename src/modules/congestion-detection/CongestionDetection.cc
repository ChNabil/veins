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
#include "CongestionDetection.h"
#include <omnetpp.h>
#include <math.h>
#include <string>

using Veins::TraCIMobilityAccess;
using Veins::AnnotationManagerAccess;
using namespace std;

Define_Module(CongestionDetection);

CongestionDetection::CongestionDetection() {
	// TODO Auto-generated constructor stub

}

CongestionDetection::~CongestionDetection() {
	// TODO Auto-generated destructor stub
}

void CongestionDetection::initialize(int stage) {
	BaseWaveApplLayer::initialize(stage);
	if (stage == 0) {
		traci = TraCIMobilityAccess().get(getParentModule());
		annotations = AnnotationManagerAccess().getIfExists();
		ASSERT(annotations);
		//EV << "\n\nOwn Traci  TEST: <"<<traci->getId()<<", " << traci->getAngleRad() << ", "<< traci->getCurrentSpeed()<<", "<<traci->getCurrentPosition().x <<", "<<traci->getCurrentPosition().y<< ">\n\n\n";
	}
	ntime = new cMessage("Timer");
	btime = new cMessage("beac");

	bool started = false;

	//static configuration
	CarID = par("vehicle_ID"); //fix 75
	compareWithTV = par("exIDtV").str(); //default LaFerrari
	L_T = par("loc_T"); //fix 1
	I_L = par("ind_L"); //fix 2
	I_TRUCK = par("ind_Truck").doubleValue(); //fix 1
	V_T = 8.3333; // fix 30

	F_TRUCK = par("f_t"); //default 18
	F_V = par("f_p");	//default 30
	sB = par("sBeacon").boolValue(); //default true

	//dynamische Variablen
	V_C = 0; // dyn 	tracyData
	V_ADD = 0; //dyn
	I_V = 0; //dyn
	B_CD = false; //dyn
	L_C_X = 0.0; //dyn	tracyData
	L_C_Y = 0.0; //dyn	tracyData

	//nicht für speed Monitoring noetig
	I_LDM = 0; //dyn
	I_M = 0; //index messages, number of tuples in Warning;
	angle = 0.0; //dyn	tracyData

	ctr =0;
	ctr = I_T = ((1 + L_T) * 60);

	//compareWithTV = "LaFerrari";
	//externalID = "LaFerrari";

	//EV << "\n\n\n\n" << externalID.compare(compareWithTV)<<"\n\n\n\n";

	CarID = traci ->getId();
	if(CarID == 75){
		scheduleAt(simTime() + 1, ntime);
		//EV  << "\n\n\n\n" << "LAFERRARI "<<"\n\n\n\n";
	}
	if(sB)
		scheduleAt(simTime() + 10, btime);
}

double CongestionDetection::calcVelocityAverage() {
	double erg = 0.0;
	erg = V_ADD / I_T;
	return erg;
}

void CongestionDetection::handleSelfMsg(cMessage *msg) {


	//update TraCI data
	//int id = traci->getId();
	CarID = traci->getId();
	externalID = traci->getExternalId();
	V_C = traci->getCurrentSpeed();
	Coord L_C = traci->getCurrentPosition();
	L_C_X = L_C.x;
	L_C_Y = L_C.y;
	angle = traci->getAngleRad();

	////////////////////////////////////
	if(msg == btime && sB)
	{
		stringstream beacon;
	beacon << CarID << "," << calcDirection() << "," << L_C_X << "," << L_C_Y
			<< "," << V_C;
	setBeaconData(beacon.str());
		WaveShortMessage *wsm = prepareWSM("beacon", beaconLengthBits, type_CCH, beaconPriority, 0, -1);
		wsm->setWsmData(beacon.str().c_str());
		sendWSM(wsm);
		scheduleAt(simTime()+10,btime);
	}
	///////////////////////////////////

	//let the congestion detection begin with car (sumo-name LaFerrari)
	if (msg == ntime && CarID == 75)
	{

			EV << "\n\n LDM.size: "<< LDM.size() <<" " << started <<" " << B_CD<<"\n\n\n";

		if (V_C <= V_T && simTime() > 170) {
			//2. in Algo starts here

			if (!started) {
				EV <<  "\n\n started: \n\n\n";
				started = true;
				V_ADD = 0;
			}
		}
		if (started && !B_CD) {

			V_ADD += V_C;
			if(ctr>=0)
				EV << "\n\n count: " << ctr << "\n\n";


			if (ctr == 0) {
				EV << "\n\n\n\n\n\n\n";
				double average = calcVelocityAverage();
				if (average <= V_T) {

					if (LDM.size() >= (I_L * ((I_TRUCK*F_TRUCK) + ((1-I_TRUCK)*F_V))) || M_T.size() > 0) {
						B_CD = true;
						stringstream s;
						s<< "LDM.size = "<<LDM.size()<<" average_Speed="<<average <<" simtime= " << simTime();
						//opp_error(s.str().c_str());
						congestionDetected();
					}

				}

			}
			ctr--;
		}
		scheduleAt(simTime() + 1, ntime);
	}else{
		started = false;
	}

}

//decides which type of message we send
void CongestionDetection::congestionDetected() {
	//3. Localize current position
	Coord L_C = traci->getCurrentPosition();
	angle = traci->getAngleRad();
	stringstream message_text;
	message_text << "<" << calcDirection() << "," << L_C.x << "," << L_C.y
			<< "," << simTime().str() << "," << L_T << ">";

	EV << "\n\n\nCongestion Detected!!! "<< I_M << "\n\n\n";
	//4.
	if (I_M == 0) {
		//5. build message and send message

		WaveShortMessage* wsm = buildWarning(message_text.str());
		sendMessage(wsm);
	} else {

		double congestionlength = calcCongestionLength();
		EV << "der Stau hat eine Laenge von " << congestionlength << " m \n";
		M_T.push_back(message_text.str());

		//5. build message and send message
		WaveShortMessage* wsm = buildWarning(M_T);
		sendMessage(wsm);
	}

}

int CongestionDetection::calcDirection() {
	//0 is East, 1 South, 2 West, 3 North;
	if (45 <= angle && angle < 135) //EAST
		return 0;
	if ((-135) <= angle && angle < -45) //West
		return 3;
	if ((-45) <= angle && angle < 45) //North
		return 2;
	return 1; //South
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////BeaconDataElement///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CongestionDetection::beaconDataElement::initialize() {
	id = -1;
	d = -1;
	locx = -1;
	locy = -1;
	velocity = -1;
}
void CongestionDetection::beaconDataElement::setId(int index) {
	id = index;
}
int CongestionDetection::beaconDataElement::getID() {
	return id;
}
void CongestionDetection::beaconDataElement::setDirection(int dir) {
	d = dir;
}
int CongestionDetection::beaconDataElement::getDir() {
	return d;
}
void CongestionDetection::beaconDataElement::setLocX(double x) {
	locx = x;
}
double CongestionDetection::beaconDataElement::getLocX() {
	return locx;
}
void CongestionDetection::beaconDataElement::setLocY(double y) {
	locy = y;
}
double CongestionDetection::beaconDataElement::getLocY() {
	return locy;
}
void CongestionDetection::beaconDataElement::setVelocity(double v) {
	velocity = v;
}
double CongestionDetection::beaconDataElement::getVelocity() {
	return velocity;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////WSM Messages// /////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//sends wsm
void CongestionDetection::sendMessage(WaveShortMessage* wsm) {
	 EV << "\n\n\n WSM \""<< wsm->getName()<< "\" has been send!! \n\n\n\n";
	sendWSM(wsm);
}
//add sending delay
void CongestionDetection::sendWSM(WaveShortMessage* wsm) {
	sendDelayedDown(wsm, individualOffset);
}

//
void CongestionDetection::onBeacon(WaveShortMessage* wsm) {
	string message = wsm->getWsmData();
	handleIncommingBeacons(message);
}

CongestionDetection::beaconDataElement CongestionDetection::parseStringToBDE(string s) {
	CongestionDetection::beaconDataElement elem;
	string tuple;
	int i = 0;
	int x = 0;
	int xold = 0;

	while (i < 5) {
		x = s.find_first_of(",", xold);
		tuple = s.substr(xold, x - xold);
		xold = x + 1;
		if (i == 0)
			elem.setId(atoi(tuple.c_str()));
		else if (i == 1)
			elem.setDirection(atoi(tuple.c_str()));
		else if (i == 2)
			elem.setLocX((atof(tuple.c_str())));
		else if (i == 3)
			elem.setLocY((atof(tuple.c_str())));
		else if (i == 4)
			elem.setVelocity(atof(tuple.c_str()));

		i++;
	}
	return elem;
}

void CongestionDetection::handleIncommingBeacons(std::string beacon) {
	string b = beacon;
	string tuple;
	int x = 0;

	x = b.find_first_of(",", 0);
	tuple = b.substr(0, x + 1);

	int id = atoi(tuple.c_str());

	bool isInList = searchIdinLDM(id);
	if (id != CarID && !isInList)
		LDM.push_back(parseStringToBDE(beacon));
	if (id != CarID && isInList) {
		int pos = searchIdinLDMPos(id);
		CongestionDetection::beaconDataElement tmp = LDM.at(pos);
		CongestionDetection::beaconDataElement neu = parseStringToBDE(beacon);

		tmp.setDirection(neu.getDir());
		tmp.setLocX(neu.getLocX());
		tmp.setLocY(neu.getLocY());
		tmp.setVelocity(neu.getVelocity());
	}
}
// searches, if the id is in LDM
bool CongestionDetection::searchIdinLDM(unsigned int id) {
	for (int i = 0; i < LDM.size(); i++) {
		if (id == LDM.at(i).getID())
			return true;
	}
	return false;
}

int CongestionDetection::searchIdinLDMPos(int id) {
	for (int i = 0; i < LDM.size(); i++) {
		if (id == LDM.at(i).getID())
			return i;
	}
	return -1;
}

//wsm data inbox for congestion warning messages
void CongestionDetection::onData(WaveShortMessage* wsm) {
	if (I_M == 0) {

		//up to now, no traffic congestion detected
		M_T = parseWarning(wsm);
		I_M = M_T.size();

		EV << "\n\n Warning received "<<I_M <<" \n\n";

	} else {
		//do nothing, cause congestion warning allready received
	}
}

//builds a list form WSM
std::vector<std::string> CongestionDetection::parseWarning(
		WaveShortMessage *msg) {
	I_M = 0;
	std::vector<std::string> list;
	string message = msg->getWsmData();
	string tuple;

	int l = message.length();
	bool isLast = false;
	int x = 0;
	int xold = 0;

	while (!isLast) {
		x = message.find_first_of(">", xold);
		tuple = message.substr(xold, x + 1 - xold);
		xold = x + 1;
		if (x + 1 == l)
			isLast = true;

		list.push_back(tuple);
		I_M++;
	}
	return list;
}

//shows list in event console
void CongestionDetection::paintList(std::vector<std::string> ls) {
	int x = ls.size();

	EV << "\n\n";
	while (x > 0) {
		string s = ls.front();
		ls.erase(ls.begin());
		EV << s << "\n";
		x--;
	}
	EV << "\n\n";

}

//builds the whole WSM, ready to send
WaveShortMessage* CongestionDetection::buildWarning(
		std::vector<std::string> list) {
	t_channel channel = dataOnSch ? type_SCH : type_CCH;
	WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel,
			dataPriority, -1, 2);
	string s = buildString(list);
	wsm->setWsmData(s.c_str());
	return wsm;
}

//builds the whole WSM, ready to send
WaveShortMessage* CongestionDetection::buildWarning(std::string s) {
	t_channel channel = dataOnSch ? type_SCH : type_CCH;
	WaveShortMessage* wsm = prepareWSM("data", dataLengthBits, channel,
			dataPriority, -1, 2);
	wsm->setWsmData(s.c_str());
	return wsm;
}

//builds string from vetor data
std::string CongestionDetection::buildString(std::vector<std::string> list) {
	int index = list.size();
	string message;
	while (index > 0) {
		string s = list.front();
		list.erase(list.begin());
		index--;
		message += s;
	}
	return message;
}


double CongestionDetection::calcCongestionLength() {
	double distance = 0;
	double locx = L_C_X;
	double locy = L_C_Y;

	if (M_T.size() == 0)
		distance = 0;
	else if (M_T.size() == 1) {
		string s = M_T.front();
		Coord value = getCoordFromString(s);
		EV << "\n\n"<<value.x << " " << value.y ;
		distance = calcLength(value.x, value.y, locx, locy);
	} else
		for (int i = 0; i < M_T.size(); i++) {
			if (i + 1 <= M_T.size()) {
				Coord fst = getCoordFromString(M_T.at(i));
				Coord snd = getCoordFromString(M_T.at(i + 1));
				distance += calcLength(fst.x, fst.y, snd.x, snd.y);
			} else {
				Coord fst = getCoordFromString(M_T.at(i));
				distance += calcLength(fst.x, fst.y, locx, locy);
			}

		}

	return distance;
}

Coord CongestionDetection::getCoordFromString(string s) {
	Coord res;
	bool isfinished = false;
	int x = 0;
	int xold = 0;
	int i = 0;
	string tuple;

	while (!isfinished) {
		x = s.find_first_of(",", xold);
		tuple = s.substr(xold, x - xold);
		xold = x + 1;
		if (i == 0) {
			i++;
		}
		if (i == 2)
			res.x = atof(tuple.c_str());
		if (i == 3) {
			isfinished = true;
			res.y = atof(tuple.c_str());
		}
		i++;

	}
	return res;
}
//calculates the distance between two points
double CongestionDetection::calcLength(double x0, double y0, double x1,
		double y1) {
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
