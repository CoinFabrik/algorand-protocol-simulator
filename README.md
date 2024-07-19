## Algorand protocol simulator

To run this simulation you need to install [`omnet++`](https://omnetpp.org/download/).

### For Ubuntu 22.04 - Errors and Solutions

If you are installing `omnet++` and following the INSTALL txt file and came accross the following error:

```bash
configure: error: Standard C math library -lm not found
```
Try

```bash
sudo apt install libstdc++-12-dev
```

OPTIONAL: 

Add "{installation home}/bin" to your PATH environment variable so that you can start omnet++ from any directory.

### Install Algorand's libsodium fork

First, you may need to install `libboost-all-dev`:
```bash
sudo apt-get install libboost-all-dev 
```
Secondly, to install Algorand's libsodium fork, you need to go to the `dependencies` folder.
Inside `algorand-protocol-simulator/dependencies/libsodium-fork` run:

```bash
sudo bash ./autogen.sh
./configure
make
sudo make install
```


### Running the simulation

Inside `algorand-protocol-simulator/src` run `make` to generate the simulation executable.

If everything goes well, you should see:
```bash
GlobalSimulationManager.cc
ParticipationNode.cc
SimulationEvent.cc
Creating executable: ../out/clang-release/src/algorand-protocol-simulator
```

To run the simulation, run the following command:

```bash
./algorand-protocol-simulator -r 0 -u Cmdenv -c General -n ../src;. omnetpp.ini
```
You should see

```bash
OMNeT++ Discrete Event Simulation  (C) 1992-2022 Andras Varga, OpenSim Ltd.
Version: 6.0.3, build: 240223-17fcae5ef3, edition: Academic Public License -- NOT FOR COMMERCIAL USE
See the license for distribution terms and warranty disclaimer

Setting up Cmdenv...

Loading NED files from ../src:  1

Preparing for running configuration General, run #0...
Redirecting output to file "../algorand-protocol-simulator/src/out.json"...

```
