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

#ifndef GLOBALSIMULATIONMANAGER_H_
#define GLOBALSIMULATIONMANAGER_H_

#include <omnetpp.h>
#include <unordered_map>
#include <deque>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <fstream>
#include <string>

#ifndef DATATYPEDEFINITIONS_H_
    #include "DataTypeDefinitions.h"
#endif

#ifndef PARTICIPATIONNODE_H_
    #include "ParticipationNode.h"
#endif

using namespace omnetpp;




struct Relay
{
    //in this model, relay nodes act as connections in between participation nodes
    //note that if there's no route between two nodes, they can't send messages to each other

    //acts as an address
    uint64_t RelayID;

    //Actual connections. IDs are positions in the over-arching network definition
    std::vector<int> RelayConnections;          //Relay node IDs in network list
    std::vector<int> ParticipationConnections;  //Participation node IDs in network list

    //connection delays
    struct ConnectionDelays
    {
        //if false, connection is down
//        bool ConnectionStatus = true;

        simtime_t InDelay;
        simtime_t OutDelay;

//        simtime_t InDelayVariance;
//        simtime_t OutDelayVariance;
    };
    std::vector<ConnectionDelays> RelayConnectionDelays;
    std::vector<ConnectionDelays> PartNodeConnectionDelays;
};




class NetworkDefinition
{
public:
    bool InitializedNetwork;

    std::vector<std::vector<int>> MatrixGraph;

    std::vector<Relay> RelayNodes;
    std::vector<class ParticipationNode*> ParticipationNodes;


    void LoadNetworkFromFile();


    NetworkDefinition(){}
    void InitNetwork(int nRelayNodes, int nPartNodes);

    void LoadPartNode(class ParticipationNode* PartNode, int PartNodeIndex, std::vector<int>& RelayConnections);
};




class GlobalSimulationManager : public cSimpleModule{
public:
    GlobalSimulationManager();
    virtual ~GlobalSimulationManager(){}
    void initialize(){}

    //singleton instance
    static GlobalSimulationManager* SimManager;


    //chronological time measuring
    std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
    std::chrono::duration<double> GetCurrentChronoTime(){return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - StartTime); }


    //represents "canonical" data
    Ledger FullLedger;
    std::unordered_map<Address, BalanceRecord> BalanceMap;


    //Network stuff
    NetworkDefinition Network;


    void PropagateMessageThroughNetwork(std::vector<int>& PendingRelayNodeVector, int SenderPartNode, void* msg, MsgType type);


    //scenario load and run data and functions
    void LoadContextFromFiles(std::string& NetworkDef_filename, std::string& CF_filename);
    void LoadEventsFromEF(std::string& filename);

    //std::vector<class SimulationEvent*> SimEventQueue;
};


#endif /* GLOBALSIMULATIONMANAGER_H_ */
