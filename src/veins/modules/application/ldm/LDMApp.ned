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

package org.car2x.veins.modules.application.ldm;
import org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer;
import org.car2x.veins.base.modules.IBaseApplLayer;

//
// Basic LDM implementation
//
// @author Rens van der Heijden
//
// @see BaseWaveApplLayer
// @see Nic80211p
// @see Mac1609_4
// @see PhyLayer80211p
//
simple LDMApp extends BaseWaveApplLayer
{
        @class(LDMApp);

        @signal[distance](type="double");
        @statistic[distanceTransmitted](title="distance from real sender"; source="distance"; record=stats; interpolationmode=none);

        @signal[distanceAttacker](type="double");
        @statistic[distanceFalsified](title="distance from attacker"; source="distanceAttacker"; record=stats; interpolationmode=none);

        @signal[attackerBeacons](type="long");
        @statistic[attackerBeacons](title="beacons received by nodes, sent by attackers"; source="attackerBeacons"; record=sum; interpolationmode=none);

        @signal[benignBeacons](type="long");
        @statistic[benignBeacons](title="beacons received by nodes, sent by attackers"; source="benignBeacons"; record=sum; interpolationmode=none);

  @signal[ARTTruePositives](type="int");
  @signal[ARTTrueNegatives](type="int");
  @signal[ARTFalsePositives](type="int");
  @signal[ARTFalseNegatives](type="int");
        @statistic[ARTTP](title="ARTTP"; source="ARTTruePositives"; record=sum; interpolationmode=none);
        @statistic[ARTTN](title="ARTTN"; source="ARTTrueNegatives"; record=sum; interpolationmode=none);
        @statistic[ARTFP](title="ARTFP"; source="ARTFalsePositives"; record=sum; interpolationmode=none);
        @statistic[ARTFN](title="ARTFN"; source="ARTFalseNegatives"; record=sum; interpolationmode=none);

  @signal[eARTTruePositives](type="int");
  @signal[eARTTrueNegatives](type="int");
  @signal[eARTFalsePositives](type="int");
  @signal[eARTFalseNegatives](type="int");
        @statistic[eARTTP](title="eARTTP"; source="eARTTruePositives"; record=sum; interpolationmode=none);
        @statistic[eARTTN](title="eARTTN"; source="eARTTrueNegatives"; record=sum; interpolationmode=none);
        @statistic[eARTFP](title="eARTFP"; source="eARTFalsePositives"; record=sum; interpolationmode=none);
        @statistic[eARTFN](title="eARTFN"; source="eARTFalseNegatives"; record=sum; interpolationmode=none);

  @signal[ExchangeTruePositives](type="int");
  @signal[ExchangeTrueNegatives](type="int");
  @signal[ExchangeFalsePositives](type="int");
  @signal[ExchangeFalseNegatives](type="int");
        @statistic[ExchangeTP](title="ExchangeTP"; source="ExchangeTruePositives"; record=sum; interpolationmode=none);
        @statistic[ExchangeTN](title="ExchangeTN"; source="ExchangeTrueNegatives"; record=sum; interpolationmode=none);
        @statistic[ExchangeFP](title="ExchangeFP"; source="ExchangeFalsePositives"; record=sum; interpolationmode=none);
        @statistic[ExchangeFN](title="ExchangeFN"; source="ExchangeFalseNegatives"; record=sum; interpolationmode=none);

  @signal[MergedTruePositives](type="int");
  @signal[MergedTrueNegatives](type="int");
  @signal[MergedFalsePositives](type="int");
  @signal[MergedFalseNegatives](type="int");
        @statistic[MergedTP](title="MergedTP"; source="MergedTruePositives"; record=sum; interpolationmode=none);
        @statistic[MergedTN](title="MergedTN"; source="MergedTrueNegatives"; record=sum; interpolationmode=none);
        @statistic[MergedFP](title="MergedFP"; source="MergedFalsePositives"; record=sum; interpolationmode=none);
        @statistic[MergedFN](title="MergedFN"; source="MergedFalseNegatives"; record=sum; interpolationmode=none);

        double ART_threshold = default(400m)   @unit(m);
        double eART_threshold = default(800m)   @unit(m);
        double Exchange_threshold = default(400m)   @unit(m);
        double eART_variance = default(100m)    @unit(m);
        double eART_max_uncertainty = default(0.5);
        double exchangeUncertaintyFactor = default(0.1);
        double maliciousProbability = default(0);
        int attackerType = default(0);
        volatile double attackerParameter = default(0m)      @unit(m);
        double detectionThreshold = default(0.5);
        bool validConfiguration = default(false);
}
