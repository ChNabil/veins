This document contains notes I wrote while using VEINS. Currently mostly an overview of what is where.
## Version
In order to run the most recent version of VEINS (3.0), see [the tutorial](http://veins.car2x.org/tutorial/).
In particular, make sure you have sumo 0.21.0 and OMNeT++ (these notes assume 4.5).
For the scripts to work correctly, the `opp_run` binary of OMNeT++ should be in the search path; in other words, `PATH` should contain the omnetpp binary folder (e.g., `/home/namnatulco/ulm/simulation/omnetpp-4.5./bin`)

## Structure
The following are some important files; read this to understand how the veins respository/directory is organized.

`./configure` -- run this to generate a Makefile in ./src/  
`./Makefile` -- the makefile that you should use to compile veins code (run ./configure first)  
`./run`, `./debug` and `./memcheck` -- python scripts that run a simulation (regularly, as debug and with valgrind)  
`./examples/veins` -- this directory contains an example:  
`./examples/veins/omnetpp.ini` -- example of an appropriate omnet++ config  
`./examples/veins/RSUExampleScenario.ned` -- example of a .ned file for a scenario.  
`./examples/veins/config.xml` -- example of a VEINS configuration; contains e.g. pathloss model, etc.  
`./examples/veins/erlangen.*.xml` and `erlangen.sumo.cfg` -- example configuration of SUMO that is expected by the example scenario  
`./examples/veins/run` -- a script that starts `opp_run` with some additional parameters (through the python script `./run`)  
`./sumo-launchd.py` -- python script that runs SUMO as needed (see --help)  
`./out` -- directory that will contain object code, libveins.so, and so on. Created by make scripts.  
`./doc` -- documentation.  
`./images` -- images that can be used by the OMNeT++ GUI.  
`./tests` -- tests.  
`./src` -- contains all code.  
`./src/base` -- contains low-level features. Mostly PHY and MAC tools, battery management, mobility basics, channel sensing.  
`./src/inet` -- (3D) Coordinate representation  
`./src/scripts` -- scripts used to generate the run/debug/memcheck tools.  
`./src/nodes` -- contains NED files for network  
`./src/modules` -- contains tools to be used for simulations:  
`./src/modules/world` -- contains visualization management for OMNeT++ GUI.  
`./src/modules/obstacle` -- contains obstacle modeling [for attenuation].  
`./src/modules/analogueModel` -- contains signal models.  
`./src/modules/phy` -- contains PHY models and PHY<->MAC interfaces.  
`./src/modules/mac` -- contains MAC models (802.11p/1609.4).  
`./src/modules/nic` -- contains a network interface for 802.11p.  
`./src/modules/mobility` -- contains mobility models (currently only TraCI).  
`./src/modules/messages` -- contains message types.  
`./src/modules/application` -- contains actual applications that run ontop of the stack (a TraCI demo app and a basic wave application layer)

## File types and conventions
### NED files
OMNeT++ files that say how classes are tied together. Typically associated with .cc and .h files of the same name. See OMNeT++ docu for more info.
### MSG files
OMNeT++ messages. C++ code is generated from these files. `cplusplus{{}}` blocks are used to add custom code.
### simulation configuration
As with the `/examples/veins` directory, it is recommended to separate configuration from code.

## Basics
### Compiling new simulation code [from the veins root directory]: 
```sh
./configure
make
```

### Running the example simulation [GUI default]:
```sh
cd ./examples/veins
#assuming SUMOPATH contains the name of the sumo executable (or add sumo's `/bin` directory to PATH)
python sumo-launchd.py -dc "$SUMOPATH"  
./run
```
This creates a results directory that contains output, which can be analyzed using various tools (amount of output depends on the simulation).
#### Command line
For command line execution (e.g., simulation server), replace the last line with (where `CONFIG` is `nodebug` or `debug` for the example sim, see the `./examples/veins/omnetpp.ini`):
```sh
./run -u Cmdenv -c "CONFIG"
```
