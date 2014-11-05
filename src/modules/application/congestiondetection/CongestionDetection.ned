package org.car2x.veins.modules.application.congestiondetection;
//
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
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;

simple CongestionDetection extends BaseWaveApplLayer
{
    @class(CongestionDetection);
	    int vehicle_ID = default(458);
	    string exIDtV = default("LaFerrari");
	    int loc_T = default(0);
	    int ind_T = default(60);
	    int ind_L = default(2);
	    double ind_Truck = default(1.0); //Test 2 0.5
	    int f_t = default(18);
	    int f_p = default(30);
	    bool sBeacon = default(true);
}