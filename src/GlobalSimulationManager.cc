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
    if (!SimManager)
    {
        SimManager = this;
        Network.InitializedNetwork = false;

        std::string net("../test_network.nf"), pl;
        LoadContextFromFiles(net, pl);
    }

    StartTime = std::chrono::high_resolution_clock::now();
}




void NetworkDefinition::InitNetwork(int nRelayNodes, int nPartNodes)
{
    RelayNodes.resize(nRelayNodes);
    ParticipationNodes.resize(nPartNodes);
    InitializedNetwork = true;


    //Fully connected
    for (int i = 0; i < nRelayNodes; i++)
        for (int h = 0; h < nRelayNodes; h++)
            if (h != i)
            {
                RelayNodes[i].RelayConnections.push_back(h);
            }
//    for (int i = 0; i < nRelayNodes; i++)
//        for (int h = 0; h < 10; h++)
//        {
//            int RelayToConnect = rand() % nRelayNodes;
//            while (RelayToConnect == i || std::find(RelayNodes[i].RelayConnections.begin(), RelayNodes[i].RelayConnections.end(), RelayToConnect) != RelayNodes[i].RelayConnections.end())
//                RelayToConnect = rand() % nRelayNodes;
//
//            RelayNodes[i].RelayConnections.push_back(RelayToConnect);
//            RelayNodes[RelayToConnect].RelayConnections.push_back(i);
//        }
}


void NetworkDefinition::LoadNetworkFromFile()
{

}


void NetworkDefinition::LoadPartNode(class ParticipationNode* PartNode, int PartNodeIndex, std::vector<int>& RelayConnections)
{
    ParticipationNodes[PartNodeIndex] = PartNode;
    for (int RelayID : PartNode->RelayConnections)
        RelayNodes[RelayID].ParticipationConnections.push_back(PartNodeIndex);
}




void GlobalSimulationManager::PropagateMessageThroughNetwork(std::vector<int>& PendingRelayNodeVector, int SenderPartNode, void* msg, MsgType type)
{
    Enter_Method_Silent("PropagateMessageThroughNetwork");

    bool VisitedRelayNodes[RELAYS] = {false};
    bool VisitedPartNodes[TOTAL_NODES] = {false};
    for (int r : PendingRelayNodeVector) VisitedRelayNodes[r] = true;

    //initializing two stacks here (they will be processed as such)
    std::deque<int> PendingRelayNodes(PendingRelayNodeVector.begin(), PendingRelayNodeVector.end());
    std::deque<int> PendingPartNodes;

    while(!PendingRelayNodes.empty())
    {
        int ProcessingRelayID = PendingRelayNodes.back();
        PendingRelayNodes.pop_back();

        VisitedRelayNodes[ProcessingRelayID] = true;

        Relay* ProcessingRelay = &Network.RelayNodes[ProcessingRelayID];
        for (int PartNodeID : ProcessingRelay->ParticipationConnections)
            if (!VisitedPartNodes[PartNodeID])
            {
                //esto es una optimizacion, pero en realidad tengo que manejar el caso de mandar repetidos
                VisitedPartNodes[PartNodeID] = true;
                PendingPartNodes.push_back(PartNodeID);
            }

        for (int RelayNodeID : ProcessingRelay->RelayConnections)
            if (!VisitedRelayNodes[RelayNodeID])
            {
                VisitedRelayNodes[RelayNodeID] = true;
                PendingRelayNodes.push_back(RelayNodeID);
            }
    }


    while(!PendingPartNodes.empty())
    {
        int PartNodeID = PendingPartNodes.back();
        PendingPartNodes.pop_back();

        switch(type)
        {
            case TXN:
                Network.ParticipationNodes[PartNodeID]->HandleTransaction(*(Transaction*)(msg));
                break;

            case PROPOSAL:
                Network.ParticipationNodes[PartNodeID]->HandleProposal(*(ProposalPayload*)(msg));
                break;

            case VOTE:
                Network.ParticipationNodes[PartNodeID]->HandleVote(*(Vote*)(msg));
//                Network.ParticipationNodes[PartNodeID]->TEST_ScheduleVoteHandling(0.05f, *(Vote*)(msg));
                break;

            case BUNDLE:
                Network.ParticipationNodes[PartNodeID]->HandleBundle(*(Bundle*)(msg));
                break;

            default:
                break;
        }
    }
}


void GlobalSimulationManager::LoadContextFromFiles(std::string& NetworkDef_filename, std::string& CF_filename)
{
    //file pointer
    std::fstream fin;

    //open an existing file
    fin.open(NetworkDef_filename, std::ios::in);

    if (!fin.is_open())
    {
        EV << "Unable to open network definition file..." << endl;
        return;
    }

    //get number of partNodes and number of relayNodes
    std::string firstLine;
    std::getline(fin, firstLine);
    std::string strN;

    std::stringstream s(firstLine);
    std::getline(s, strN, ' ');
    int nPartNodes = std::stoi(strN);

    std::getline(s, strN, ' ');
    int nRelayNodes = std::stoi(strN);


    //get all partnode to relay connections (opt: with in delay and out delay)
    for (int i = 0; i < nPartNodes; i++)
    {
        std::string line;
        std::getline(fin, line);

        //in line, I have all connections of the i-th partNode

    }


    fin.close();

    //get all relay to relay connections (with in delay and out delay)
    //TODO




//    for (std::string line; std::getline(fin, line);)
//    {
//
//
//    }

//    vector<std::string> row;
//    std::string line, word, temp;
//    while(fin >> temp)
//    {
//        //skip first row as its the header
//        row.clear();
//    }


    Network.InitNetwork(nRelayNodes, nPartNodes);
}
