This document contains notes I wrote while using VEINS. Currently mostly an overview of what is where.
## Version
In order to run the most recent version of VEINS (4a1), see [the tutorial](http://veins.car2x.org/tutorial/).
In particular, make sure you have sumo 0.21.0 and OMNeT++ (these notes assume 4.5).
For the scripts to work correctly, the `opp_run` binary of OMNeT++ should be in the search path; in other words, `PATH` should contain the omnetpp binary folder (e.g., `/home/namnatulco/ulm/simulation/omnetpp-4.5./bin`). It is also helpful to define SUMOPATH as the path to your sumo binary. If you want to use C++11 features, make sure to set OMNETPP_CONFIGFILE to Makefile.inc (included in this repository)

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
`./out` -- directory that will contain object code and so on. Created by make scripts.  
`./doc` -- documentation.  
`./images` -- images that can be used by the OMNeT++ GUI.  
`./tests` -- tests.  
`./src` -- contains all code.  
`./src/scripts` -- scripts used to generate the run/debug/memcheck tools.  
`./src/veins/base` -- contains low-level features. Mostly PHY and MAC tools, battery management, mobility basics, channel sensing.  
`./src/veins/inet` -- (3D) Coordinate representation  
`./src/veins/nodes` -- contains NED files for network  
`./src/veins/modules` -- contains tools to be used for simulations:  
`./src/veins/modules/world` -- contains visualization management for OMNeT++ GUI.  
`./src/veins/modules/obstacle` -- contains obstacle modeling [for attenuation].  
`./src/veins/modules/analogueModel` -- contains signal models.  
`./src/veins/modules/phy` -- contains PHY models and PHY<->MAC interfaces.  
`./src/veins/modules/mac` -- contains MAC models (802.11p/1609.4).  
`./src/veins/modules/nic` -- contains a network interface for 802.11p.  
`./src/veins/modules/mobility` -- contains mobility models (currently only TraCI).  
`./src/veins/modules/messages` -- contains message types.  
`./src/veins/modules/application` -- contains actual applications that run ontop of the stack (a TraCI demo app and a basic wave application layer)

## File types and conventions
### NED files
OMNeT++ files that say how classes are tied together. Typically associated with .cc and .h files of the same name. See OMNeT++ docu for more info. For vim users: there is a syntax file [here](https://github.com/vim-scripts/OMNeTpp4.x-NED-Syntax-file); if you've never installed a vim syntax file, refer to [the documentation](http://vimdoc.sourceforge.net/htmldoc/syntax.html#:syn-files).

### MSG files
OMNeT++ messages. C++ code is generated from these files. `cplusplus{{}}` blocks are used to add custom code. For vim users: there is a syntax file for NEDs [here](https://github.com/vim-scripts/OMNeTpp4.x-NED-Syntax-file), which also works for MSGs; if you've never installed a vim syntax file, refer to [the documentation](http://vimdoc.sourceforge.net/htmldoc/syntax.html#:syn-files).

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
python ../../sumo-launchd.py -dc "$SUMOPATH"  
./run
```
This creates a results directory that contains output, which can be analyzed using various tools (amount of output depends on the simulation).
#### Command line
For command line execution (e.g., simulation server), replace the last line with (where `CONFIG` is `nodebug` or `debug` for the example sim, see the `./examples/veins/omnetpp.ini`):
```sh
./run -u Cmdenv -c "CONFIG"
```

### Creating a new simulation
Use `./createsim my-sim` to create a simulation (or refer to the `createsim` script), which copies certain configuration files and scripts from `./examples/veins`. It also creates symbolic links to the relevant map and configuration files for SUMO. Of course, you can remove these and use your own SUMO configurations (make sure to update the OMNeT++ configuration). Just update this line in `omnetpp.ini` (see also the explanation below) to your .launchd.xml file:
```
*.manager.launchConfig = xmldoc("erlangen.launchd.xml")
```

#### omnetpp.ini
This is the central configuration file for OMNeT++. Here you can define simulation types, control parameters passed to NED files and so on. You can use this to vary the number of nodes or parameterize your algorithms. Be sure to read this file thoroughly and [check out the OMNeT++ documentation](http://www.omnetpp.org/doc/omnetpp/manual/usman.html#sec329) for more information regarding the syntax.

The simulation is organized hierarchically (in this case the `RSUExampleScenario` being the root), which means that you will configure the different nodes and their components using a tree-like structure. All parameters given directly to OMNeT++ (such as `network=..`) are on the top layer, while parameters for VEINS components are typically defined indirectly, such as the size of the simulated area: `*.playgroundSizeX`. Definitions that apply to different types of nodes can be matched with `**`, for example; `***.nic....` configures the network interfaces of any node. Finally, you can configure specific nodes by hand using `*.rsu[0]...` (to configure the 0th RSU node), and all nodes using `*.rsu[*]....`.

#### RSUExampleScenario.ned
This is the example scenario defined for the VEINS demo. It shows how to build your Scenario as a NED file by extending the standard scenario (`./src/veins/nodes/Scenario.ned`). In this case, the only change is the addition of an RSU. If you check out the super-class, you'll find (for example) how playground size (= size of the simulated area) is configured.

#### config.xml
In `config.xml`, you can define certain parameters of VEINS' default NIC here, including the selected path loss model.

### Beaconing
When implementing beaconing applications, you will need to make sure that you avoid the nodes synchronizing their transmission unintentionally, because you might configure sending every second starting at `t=0.1`. This will cause all vehicles to attempt to send at exactly this time; in practice this will not happen, and you thus need to compensate for this. For example, you can add some random jitter to the first transmission, which breaks the synchronization. See also [this thread](https://groups.google.com/forum/#!topic/omnetpp/EhumLHlW3l0).

## Errors
Sometimes, gcc/opp error messages are less than clear. These are the messages I've encountered problems with. 

### Missing symbols
Situation: your code compiles, but trying to run it with OMNeT++ causes errors similar to the following:  
```
<!> Warning: opp_run: Cannot check library ../../src/veins: ../../src//libveins.so: undefined symbol: _ZN6LDMApp10prepareWSMESsi9t_channeliii
<!> Error during startup: Cannot load library '../../src//libveins.so': ../../src//libveins.so: undefined symbol: _ZN6LDMApp10prepareWSMESsi9t_channeliii.
```
The relevant C++ code for this case is:
```cpp
class LDMApp : public BaseWaveApplLayer {
//...
virtual WaveShortMessage* prepareWSM(std::string name, int dataLengthBits, t_channel channle, int priority, int rcvId, int serial=0) override;
//...
}
```
Which should override the standard VEINS method `BaseWaveApplLayer::prepareWSM`. Note that the `override` here is a C++11 feature (which works as expected with a current g++ compiler -- it checks that something is actually overridden; if not, it will provide an error). What happened in this particular case is that the implementation of the class `LDMApp` did not have an implementation of the specified virtual method, `prepareWSM`. Unlike what you might expect, this is **not** checked by the compiler.

### Cannot access fields from `.msg`-generated type
If you try to access a field in a Message type that was specified using the OMNeT++ `.msg` file type (See [message definitions](http://www.omnetpp.org/doc/omnetpp/manual/usman.html#sec224) in the OMNeT++ user manual) as follows (the message type is defined [here](src/veins/modules/messages/WaveShortMessage.msg)), it will not work:
```cpp
void LDMApp::onBeacon(WaveShortMessage * wsm){ 
        do_things(wsm->senderAddress);
}
```
This is intentional -- `opp_msgc` creates getters and setters for each of the variables in the message spec, and renames them to discourage direct access to these values. You should use `getSenderAddress` (in this case) to access the variables.

### Module interfaces
If you encounter the following error:
```
Error: base type org.car2x.veins.modules.application.ieee80211p.BaseWaveApplLayer is expected to be a module interface, at .../src/veins/modules/application/ldm/LDMApp.ned:35.
```
Then you tried to use the `like` keyword to extend (in this example) [BaseWaveApplLayer](src/veins/modules/application/ieee802.11p/BaseWaveApplLayer.ned) with something like this code:
```
simple LDMApp like BaseWaveApplLayer
{
}
```
You should use the `extends` keyword.

### C++11 features
See [this blog post](http://adamcavendish.is-programmer.com/posts/42316.html). A correct `Makefile.inc` is included in the root of this repository. 

### Check consistency between `omnetpp.ini` and your `.ned` files
Remember that OMNeT++ will/can not verify that all properties in your `omnetpp.ini` exist. The opposite is often true if you inherit from existing code -- for most application `.ned` files, defaults are set, and these may interact with your `omnetpp.ini`. For example, you might set your implementation of `org.car2x.veins.base.mdoules.IBaseApplLayer`'s `dataOnSch` to `true`, while your `org.car2x.veins.modules.mac.iee80211p.Mac1609_4` has its `useServiceChannel` set to `false`. This will lead to potentially confusing assertion errors!

## Debugging
There are many ways to debug your VEINS application, and many of them can be combined to get to your problem the fastest way. This section is a (mosty likely) incomplete description of the available options.

Further reading:
 * [OMNeT++ Wiki -- C++ debugging tips](http://www.omnetpp.org/pmwiki/index.php?n=Main.CplusplusDebuggingTips)
 * [GDB Manual](http://davis.lbl.gov/Manuals/GDB/gdb.html)
 * [brief GDB overview](http://phoenix.goucher.edu/~kelliher/cs318/gdb.html)

### Using `gdb` or another generic debugger
If you are familiar with C/C++ programming on Linux systems, you'll probably want to use your favorite debugger with OMNeT++ too. In fact, the `./debug` script included with VEINS uses `gdb` by default (which you can change by replacing it in the python script). You can even combine it with the Tkenv GUI: the OMNeT++ simulator allows you to jump to the debugger for the next event using a menu option. Additionally, if you set the option `debug-on-errors` in `omnetpp.ini` to `true`, you will end up in a debugger if an error (such as an assertion failure or another unhandled exception) is thrown.

In `gdb`'s debug shell, you can do the usual operations; refer to a gdb introduction if you are not familiar with these. Of particular use are the commands `bt full`, `frame`, `print` and `break` (brief descriptions are available using `help command`). 

### Using `ev<<` and the OMNeT++ GUI

At every point in your code, you can write to the `ev` file pointer. As demonstrated by some VEINS source code, it is helpful to create a macro to improve the readability of the log, like so:
```cpp
#define deciderEV (ev.isDisabled()||!debug) ? ev : ev << "[Host " << myIndex << "] - PhyLayer(Decider): "
```
As you'll notice, there are several variables in `omnetpp.ini` that influence debugging through these macros:
```
**.debug = true
**.coreDebug = true
*.annotations.draw = true
```
To debug applications, you should use the `debug` option. `*.annotations.draw` draws objects from SUMO into your GUI. This significantly slows down your simulations, so unless you're working with something like ray tracing you probably want to turn this feature off.
