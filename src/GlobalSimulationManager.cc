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

#include "GlobalSimulationManager.h"


Define_Module(GlobalSimulationManager);

//initialize singleton pointer
GlobalSimulationManager* GlobalSimulationManager::SimManager = nullptr;


GlobalSimulationManager::GlobalSimulationManager()
{
    if (!SimManager) SimManager = this;
}




void NetworkDefinition::InitNetwork(int nRelayNodes, int nPartNodes)
{

}


void NetworkDefinition::PropagateMessageThroughNetwork(std::vector<int>& PendingRelayNodeVector, int SenderPartNode, void* msg, MsgType type)
{
    //init visited flag lists
//    bool VisitedRelayNodes[RelayNodes.size()] = {false};
//    bool VisitedPartNodes[ParticipationNodes.size()] = {false};
    bool VisitedRelayNodes[200] = {false};
    bool VisitedPartNodes[500] = {false};
    VisitedPartNodes[SenderPartNode] = true;
    for (int r : PendingRelayNodeVector) VisitedRelayNodes[r] = true;

    return;

    //initializing two stacks here (they will be processed as such)
    std::deque<int> PendingRelayNodes(PendingRelayNodeVector.begin(), PendingRelayNodeVector.end());
    std::deque<int> PendingPartNodes;
    while(!PendingRelayNodes.empty())
    {
        int ProcessingRelayID = PendingRelayNodes.back();
        PendingRelayNodes.pop_back();

        Relay* ProcessingRelay = &RelayNodes[ProcessingRelayID];
        for (int PartNodeID : ProcessingRelay->ParticipationConnections)
            if (!VisitedPartNodes[PartNodeID])
            {
                //esto es una optimizacion, pero en realidad tengo que manejar el caso de mandar repetidos
                //VisitedPartNodes[PartNodeID] = true;
                PendingPartNodes.push_back(PartNodeID);
            }

        for (int RelayNodeID : ProcessingRelay->RelayConnections)
            if (!VisitedRelayNodes[RelayNodeID])
            {
                VisitedRelayNodes[RelayNodeID] = true;
                PendingRelayNodes.push_back(RelayNodeID);
            }
        VisitedRelayNodes[ProcessingRelayID] = true;
    }

    while(PendingPartNodes.size())
    {
        int PartNodeID = PendingPartNodes.back();
        PendingPartNodes.pop_back();

        //process part node

    }


    //HACE FALTA la matriz de representación para esto? quizas mejora la localidad de datos? Los limites de iteracion son bajitos, no se si vale la pena
}
