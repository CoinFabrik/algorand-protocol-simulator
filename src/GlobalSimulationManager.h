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

#ifndef DATATYPEDEFINITIONS_H_
    #include "DataTypeDefinitions.h"
#endif

using namespace omnetpp;




//struct NetworkState
//{
//    NetworkState(){}
//    NetworkState(int nRelays, int nPartNodes)
//    {
//        MatrixGraphRepresentation.resize(nRelays+nPartNodes);
//        for (auto& v : MatrixGraphRepresentation)
//            v.resize(nRelays);
//
//        //every partNode will connect to 4 relays
//        for (int i = 0; i < nPartNodes; i++)
//        {
//            int relayToConnect1 = rand()%nRelays;
//            int relayToConnect2 = rand()%nRelays;
//            int relayToConnect3 = rand()%nRelays;
//            int relayToConnect4 = rand()%nRelays;
//        }
//
//        //every relay will connect to 10
//    }
//
//    std::vector<std::vector<uint8_t>> MatrixGraphRepresentation;
//};




class NetworkDefinition
{
public:
    std::vector<Relay> RelayNodes;
    std::vector<class ParticipationNode*> ParticipationNodes;

    NetworkDefinition(){}
    void InitNetwork(int nRelayNodes, int nPartNodes);


    void PropagateMessageThroughNetwork(std::vector<int>& PendingRelayNodeVector, int SenderPartNode, void* msg, MsgType type);
};




class GlobalSimulationManager : public cSimpleModule{
public:
    GlobalSimulationManager();
    virtual ~GlobalSimulationManager(){}
    void initialize(){}


    //singleton instance
    static GlobalSimulationManager* SimManager;


    //represents "canonical" data
    Ledger FullLedger;
    std::unordered_map<Address, BalanceRecord> BalanceMap;


    //Network stuff
    NetworkDefinition Network;




//    //Stat collection
//    struct NodeRoundStats
//    {
//        uint64_t StartSimTime;
//        uint64_t EndSimTime;
//
//        //CONFIRMED BLOCK?
//    };
//    std::vector<std::vector<NodeRoundStats>> PerRoundStats;
};


#endif /* GLOBALSIMULATIONMANAGER_H_ */
