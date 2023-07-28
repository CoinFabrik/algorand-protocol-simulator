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
        LastTxnID = 0;
    }
}


void GlobalSimulationManager::initialize()
{
    StartTime = std::chrono::high_resolution_clock::now();

    std::string net("../test_network.nf"), pl;
    LoadContextFromFiles(net, pl);
}




void NetworkDefinition::InitNetwork(int nPartNodes, int nRelayNodes, std::vector<std::vector<int>>& PartNodeConnections, std::vector<std::vector<int>>& RelayNodeConnections)
{
    ParticipationNodes.resize(nPartNodes);
    RelayNodes.resize(nRelayNodes);
    InitializedNetwork = true;


//    for (int PartID = 0; PartID < PartNodeConnections.size(); PartID++)
//    {
//        auto& ConnectionList = PartNodeConnections[PartID];
//        for (int ConID = 0; ConID < ConnectionList.size(); ConID+=3)
//        {
//            int RelayID = ConnectionList[ConID];
//            RelayNodes[RelayID].ParticipationConnections.push_back(PartID);
//
//            NetworkDefinition::ConnectionDelay delay(connectionList[ConID+1], connectionList[ConID+2]);
//            RelayNodes[RelayID].PartNodesConnectionDelays.push_back(delay);
//
//            //ver si ya estan creados!
//            ParticipationNodes[PartID]->RelayConnections.push_back(RelayID);
//        }
//    }
//
//
//    for (int RelayID = 0; RelayID < RelayNodeConnections.size(); RelayID++)
//    {
//        auto& ConnectionList = RelayNodeConnections[RelayID];
//        for (int ConID = 0; ConID < ConnectionList.size(); ConID+=3)
//        {
//            int PeerRelayID = ConnectionList[RelayID];
//            RelayNodes[RelayID].RelayConnections.push_back(PeerRelayID);
//
//            NetworkDefinition::ConnectionDelay delay(connectionList[ConID+1], connectionList[ConID+2]);
//            RelayNodes[RelayID].RelayConnectionDelays.push_back(delay);
//        }
//    }


    //Fully connected
//    for (int i = 0; i < nRelayNodes; i++)
//        for (int h = 0; h < nRelayNodes; h++)
//            if (h != i)
//            {
//                RelayNodes[i].RelayConnections.push_back(h);
//            }
    for (int i = 0; i < nRelayNodes; i++)
        for (int h = 0; h < 10; h++)
        {
            int RelayToConnect = rand() % nRelayNodes;
            while (RelayToConnect == i || std::find(RelayNodes[i].RelayConnections.begin(), RelayNodes[i].RelayConnections.end(), RelayToConnect) != RelayNodes[i].RelayConnections.end())
                RelayToConnect = rand() % nRelayNodes;

            RelayNodes[i].RelayConnections.push_back(RelayToConnect);
            RelayNodes[RelayToConnect].RelayConnections.push_back(i);
        }
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

    std::deque<float> PendingPartNodeConnectionDelay;
    std::deque<float> PendingRelayNodeConnectionDelay;


    float AcumRelayDelay = 0.f;
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

        float FinalDelay = 0.f;
        switch(type)
        {
            case TXN:
                FinalDelay = DATASIZE_DELAY_MULTIPLIER_TXN * 0.f + COMPUTATION_DELAY_DELTA_TXN_HANDLER;
//                Network.ParticipationNodes[PartNodeID]->HandleTransaction(*(Transaction*)(msg));
                Network.ParticipationNodes[PartNodeID]->TEST_ScheduleTxnHandling(FinalDelay, *(Transaction*)(msg));
                break;

            case PROPOSAL:
                FinalDelay = DATASIZE_DELAY_MULTIPLIER_PROPOSAL * 0.5f + COMPUTATION_DELAY_DELTA_PROPOSAL_HANDLER;
//                Network.ParticipationNodes[PartNodeID]->HandleProposal(*(ProposalPayload*)(msg));
                Network.ParticipationNodes[PartNodeID]->TEST_ScheduleProposalHandling(FinalDelay, *(ProposalPayload*)(msg));
                break;

            case VOTE:
                FinalDelay = DATASIZE_DELAY_MULTIPLIER_VOTE * 0.5f + COMPUTATION_DELAY_DELTA_VOTE_HANDLER;
//                Network.ParticipationNodes[PartNodeID]->HandleVote(*(Vote*)(msg));
                Network.ParticipationNodes[PartNodeID]->TEST_ScheduleVoteHandling(FinalDelay, *(Vote*)(msg));
                break;

            case BUNDLE:
                FinalDelay = DATASIZE_DELAY_MULTIPLIER_BUNDLE * 0.f + COMPUTATION_DELAY_DELTA_BUNDLE_HANDLER;
//                Network.ParticipationNodes[PartNodeID]->HandleBundle(*(Bundle*)(msg));
                Network.ParticipationNodes[PartNodeID]->TEST_ScheduleBundleHandling(FinalDelay, *(Bundle*)(msg));
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

        //TODO: structure checks, sanity checks, etc.

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



    //TODO: LEVANTAR DELAYS Y PONERLOS DONDE CORRESPONDE



//    //get all partnode to relay connections (opt: with in delay and out delay)
//    for (int i = 0; i < nPartNodes; i++)
//    {
//        std::string line;
//        std::getline(fin, line);
//
//        //in line, I have all connections of the i-th partNode
//        s = std::stringstream(line);
//        while(std::getline(s, strN, ' '))
//        {
//
//        }
//    }
//
//
//    fin.close();
//
//    //get all relay to relay connections (with in delay and out delay)
//    //TODO
//
//
//
//
////    for (std::string line; std::getline(fin, line);)
////    {
////
////
////    }
//
////    vector<std::string> row;
////    std::string line, word, temp;
////    while(fin >> temp)
////    {
////        //skip first row as its the header
////        row.clear();
////    }


    std::vector<std::vector<int>> connections;
    Network.InitNetwork(nPartNodes, nRelayNodes, connections, connections);
}
