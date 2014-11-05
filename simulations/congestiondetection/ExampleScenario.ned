package simulations.congestiondetection;

import org.car2x.veins.nodes.RSU;
import org.car2x.veins.nodes.Car;
import org.car2x.veins.nodes.Scenario;

network ExampleScenario extends Scenario
{
    submodules:
       rsu[1]: RSU {
           @display("p=150,140;b=10,10,oval");
        }
}
