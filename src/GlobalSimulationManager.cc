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

//initialize singleton pointer and ID counters
GlobalSimulationManager* GlobalSimulationManager::SimManager = nullptr;
uint64_t Transaction::NEXT_ID = 0;
uint64_t Vote::NEXT_ID = 0;
uint64_t ProposalPayload::NEXT_ID = 0;
uint64_t LedgerEntry::NEXT_ID = 0;
uint64_t Bundle::NEXT_ID = 0;
LedgerEntry LedgerEntry::EMPTY_BLOCK;


GlobalSimulationManager::GlobalSimulationManager()
{
    if (!SimManager) SimManager = this;

    NumberOfAccounts = 0;
    NumberOfOnlineAccounts = 0;

    TotalMicroalgos = 0;
    TotalStakedMicroalgos = 0;

    LedgerEntry::EMPTY_BLOCK.LedgerEntryID = 0;
}


void GlobalSimulationManager::initialize()
{
    std::string net("../test_network.nf"), bal("../test_balance.bf"), pl;
    LoadContextFromFiles(net, bal, pl);


//    scheduleAfter(3.39f, new cMessage(nullptr, 0));
//    scheduleAfter(150.f, new cMessage(nullptr, 1));
//    for (float i = 0.f; i < 200.f; i+= 8.f)
//    {
//        scheduleAfter(i+4.f, new cMessage(nullptr, 0));
//        scheduleAfter(i+8.f, new cMessage(nullptr, 1));
//    }
    scheduleAfter(999, new cMessage(nullptr, 255));

    StartTime = std::chrono::high_resolution_clock::now();
}


void GlobalSimulationManager::AddParticipationNode()
{
    cModule* parentmod = getParentModule();
    int size = parentmod->getSubmoduleVectorSize("PartNode");
    parentmod->setSubmoduleVectorSize("PartNode", size+1);

    cModuleType* nodeType = cModuleType::get("ParticipationNode");
//    if (!nodeType) error("Module Type \"%s\" not found", type.c_str());

    int nodeID = Network.ParticipationNodes.size();
    ParticipationNode* node = (ParticipationNode*)(nodeType->create("PartNode", parentmod, size));
    node->NodeID = nodeID;
    Network.ParticipationNodes.push_back(node);
}




void NetworkDefinition::InitNetwork(int nPartNodes, int nRelayNodes, std::vector<std::vector<int>>& PartNodeConnections, std::vector<std::vector<int>>& RelayNodeConnections)
{
    RelayNodes.resize(nRelayNodes);

    for (int PartID = 0; PartID < PartNodeConnections.size(); PartID++)
    {
        std::vector<int>& LineConnections = PartNodeConnections[PartID];
        for (int idx = 0; idx < LineConnections.size(); idx+=3)
        {
            int RelayID = LineConnections[idx];
            RelayNodes[RelayID].ParticipationConnections.push_back(PartID);
            RelayNodes[RelayID].PartNode_InConnectionDelays.push_back(LineConnections[idx+1]);
            RelayNodes[RelayID].PartNode_OutConnectionDelays.push_back(LineConnections[idx+2]);

            ParticipationNodes[PartID]->RelayConnections.push_back(RelayID);
        }
    }


    for (int RelayID = 0; RelayID < RelayNodeConnections.size(); RelayID++)
    {
        std::vector<int>& LineConnections = RelayNodeConnections[RelayID];
        for (int idx = 0; idx < LineConnections.size(); idx+=3)
        {
            int ConRelID = LineConnections[idx];
            RelayNodes[RelayID].RelayConnections.push_back(ConRelID);

            RelayNodes[RelayID].Relay_InConnectionDelays.push_back(LineConnections[idx+1]);
            RelayNodes[RelayID].Relay_OutConnectionDelays.push_back(LineConnections[idx+2]);
        }
    }
}




void GlobalSimulationManager::PropagateMessageThroughNetwork(std::vector<int>& PendingRelayNodeVector, int SenderPartNode, void* msg, MsgType type)
{
    Enter_Method_Silent("PropagateMessageThroughNetwork");

    std::vector<bool> VisitedRelayNodes(Network.RelayNodes.size(), false);
    std::vector<bool> VisitedPartNodes(Network.ParticipationNodes.size(), false);
    for (int r : PendingRelayNodeVector) VisitedRelayNodes[r] = true;

    //initializing two stacks here (they will be processed as such)
    std::vector<int> PendingRelayNodes(PendingRelayNodeVector.begin(), PendingRelayNodeVector.end());
    std::vector<int> PendingPartNodes;


    std::vector<float> PendingPartNodeConnectionDelay;
    std::vector<float> PendingRelayNodeConnectionDelay;
    std::vector<float> AccumRelayDelay;


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

                Network.ParticipationNodes[PartNodeID]->ScheduleTxnHandling(FinalDelay, (Transaction*)(msg));
                break;

            case PROPOSAL:
                FinalDelay = DATASIZE_DELAY_MULTIPLIER_PROPOSAL * 0.0f + COMPUTATION_DELAY_DELTA_PROPOSAL_HANDLER;
//                Network.ParticipationNodes[PartNodeID]->HandleProposal(*(ProposalPayload*)(msg));

//                Network.ParticipationNodes[PartNodeID]->ScheduleProposalHandling(FinalDelay, AddCirculatingProposal((ProposalPayload*)(msg)));
                Network.ParticipationNodes[PartNodeID]->ScheduleProposalHandling(FinalDelay, (ProposalPayload*)(msg));
                break;

            case VOTE:
                FinalDelay = DATASIZE_DELAY_MULTIPLIER_VOTE * 0.0f + COMPUTATION_DELAY_DELTA_VOTE_HANDLER;  //VER! BUG CON 2.f (corregido?)
//                Network.ParticipationNodes[PartNodeID]->HandleVote(*(Vote*)(msg));

                Network.ParticipationNodes[PartNodeID]->ScheduleVoteHandling(FinalDelay, (Vote*)(msg));
                break;

            case BUNDLE:
                FinalDelay = DATASIZE_DELAY_MULTIPLIER_BUNDLE * 0.f + COMPUTATION_DELAY_DELTA_BUNDLE_HANDLER;
//                Network.ParticipationNodes[PartNodeID]->HandleBundle(*(Bundle*)(msg));

                Network.ParticipationNodes[PartNodeID]->ScheduleBundleHandling(FinalDelay, *(Bundle*)(msg));
                break;

            default:
                break;
        }
    }
}


Vote* GlobalSimulationManager::AddCirculatingVote(Vote* vt)
{
    auto entry = ObservedCirculatingVotes[vt->r].emplace(vt->voteID, *vt);
    return &((*entry.first).second);
}


ProposalPayload* GlobalSimulationManager::AddCirculatingProposal(ProposalPayload* pp)
{
    auto entry = ObservedCirculatingProposals[pp->RoundOfAssembly].emplace(pp->ProposalPayloadID, *pp);
    return &((*entry.first).second);
}


//Bundle* GlobalSimulationManager::AddCirculatingBundle(Bundle* b)
//{
//    if(!b->votes.size()) return nullptr;
//
//    auto entry = ObservedCirculatingBundles[b->votes[0]->r].emplace(b->bundleID, *b);
//    return &((*entry.first).second);
//}


void GlobalSimulationManager::LoadContextFromFiles(std::string& NetworkDef_filename, std::string& BalanceDef_filename, std::string& CF_filename)
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
    uint64_t nPartNodes = std::stoull(strN);


    std::getline(s, strN, ' ');
    uint64_t nRelayNodes = std::stoull(strN);


    //Create all required initial participation nodes
    for (int i = 0; i < nPartNodes; i++)
        AddParticipationNode();


    //get all partnode to relay connections (opt: with in delay and out delay)
    std::vector<std::vector<int>> PartNodeConnections;
    for (int i = 0; i < nPartNodes; i++)
    {
        std::vector<int> LineConnections;

        std::string line;
        std::getline(fin, line);

        //in line, I have all connections of the i-th partNode
        s = std::stringstream(line);
        while(std::getline(s, strN, ' '))
            LineConnections.push_back(std::stoi(strN));

        PartNodeConnections.push_back(LineConnections);
    }


    //get all relay to relay connections (opt: with in delay and out delay)
    std::vector<std::vector<int>> RelayConnections;
    for (int i = 0; i < nRelayNodes; i++)
    {
        std::vector<int> LineConnections;

        std::string line;
        std::getline(fin, line);

        //in line, I have all connections of the i-th relay node
        s = std::stringstream(line);
        while(std::getline(s, strN, ' '))
            LineConnections.push_back(std::stoi(strN));

        RelayConnections.push_back(LineConnections);
    }

    fin.close();
    Network.InitNetwork(nPartNodes, nRelayNodes, PartNodeConnections, RelayConnections);




    //Balance file init
    fin.open(BalanceDef_filename, std::ios::in);
    if (!fin.is_open())
    {
        EV << "Unable to open balance definition file..." << endl;
        //finalize simulation with error code
        return;
    }

    while(std::getline(fin, firstLine))
    {
        std::stringstream t(firstLine);
        std::getline(t, strN, ' ');
        uint64_t AccountAddress = std::stoull(strN);

        std::getline(t, strN, ' ');
        uint64_t AccountBalance = std::stoull(strN);

        std::getline(t, strN, ' ');
        bool AccountStatus = std::stoi(strN);

        std::vector<int> PartNodes;
        while(std::getline(t, strN, ' '))
            PartNodes.push_back(std::stoi(strN));

        AddAccountRecord(AccountAddress, AccountBalance, AccountStatus, PartNodes);
    }

    fin.close();
}


void GlobalSimulationManager::AddAccountRecord(uint64_t address, uint64_t balance, bool status, std::vector<int>& partnodes)
{
    BalanceMap[address] = BalanceRecord(balance, status);

    NumberOfAccounts++;
    TotalMicroalgos += balance;

    if (status)
    {
        TotalStakedMicroalgos += balance;
        NumberOfOnlineAccounts++;

        for (int id : partnodes)
            Network.ParticipationNodes[id]->onlineAccounts.push_back(Account(address, balance, &BalanceMap[address]));
    }
    else for (int id : partnodes) Network.ParticipationNodes[id]->offlineAccounts.push_back(Account(address, balance));
}


void GlobalSimulationManager::NodeStartedNewRound(ParticipationNode* caller, uint64_t RoundStarted)
{
    bool TodosPasaronRound = true;
    int NPasaronRound = 0;
    for (ParticipationNode* pn : Network.ParticipationNodes)
    {
        if (pn->round < RoundStarted)
        {
            TodosPasaronRound = false;
        }
        else NPasaronRound++;
    }


    if (TodosPasaronRound)
    {
        ObservedCirculatingVotes[RoundStarted-1].clear();
        ObservedCirculatingVotes.erase(RoundStarted-1);

        ObservedCirculatingProposals[RoundStarted-1].clear();
        ObservedCirculatingProposals.erase(RoundStarted-1);

//        ObservedCirculatingBundles[RoundStarted-1].clear();
//        ObservedCirculatingBundles.erase(RoundStarted-1);
        //cleanup
//        for (Transaction* txn: LivingCirculatingTxns)
    }

    if (NPasaronRound > getNumberOfPartNodes()/2 && RoundStarted > CurrentGlobalRound)
    {
        CurrentGlobalRound = RoundStarted;

        //hago esto para considerar la posibilidad de un fork
//        std::unordered_map<uint64_t, uint64_t> CandidateHashToConfirmedMap;

        GlobalLedger.Entries.push_back(caller->Ledger.Entries.back());
        UpdateBalanceMap(&GlobalLedger.Entries.back());

        EV << "G " << CurrentGlobalRound << " " << GlobalLedger.Entries.back().LedgerEntryID << " "<< simTime() << " " << std::setprecision(15) << GetCurrentChronoTime().count() << endl;
    }
}


void GlobalSimulationManager::UpdateBalanceMap(LedgerEntry* e)
{
    //VER mover los parametros despues
    auto BalanceLookback = 2*2*80;


    //update all balances according to last block
    for (Transaction* txn : e->Txns)
    {
        //por ahora solo pay
        GlobalSimulationManager::SimManager->BalanceMap[txn->Sender].RawBalance -= txn->Fee + txn->Amount;
        GlobalSimulationManager::SimManager->BalanceMap[txn->Receiver].RawBalance += txn->Amount;
        //Balance[FEE_SINK_ADDRESS].RawBalance += txn.Fee;
    }

    if (GlobalLedger.Entries.size()>BalanceLookback)
    {
        //updatear all cached balances according to lookback block
        auto& LookbackBlock =  *(GlobalLedger.Entries.end() - 1 - BalanceLookback);
        for (Transaction* txn : LookbackBlock.Txns)
        {
            //por ahora solo pay
            GlobalSimulationManager::SimManager->BalanceMap[txn->Sender].OldBalance -= txn->Fee + txn->Amount;
            GlobalSimulationManager::SimManager->BalanceMap[txn->Receiver].OldBalance += txn->Amount;
        }
    }
}




void GlobalSimulationManager::ScheduleTimedSimulationEvents()
{
//    for (SimulationEvent* ev : SimEventQueue)
//    {
//        auto* t_ev = (TimeTriggeredSimulationEvent*)(ev);
//        scheduleAfter(t_ev->TriggerTime, t_ev->SchedulingMessage);
//    }
}


void GlobalSimulationManager::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) //timeout event
    {
        if (msg->getKind() == 2)
        {
//            SimulateRandomPayTransaction();
            cancelAndDelete(msg);
        }
        else if (msg->getKind() == 0)
        {
            PartitionSimulationEvent pEv = PartitionSimulationEvent();
            pEv.ExecuteEvent();

            cancelAndDelete(msg);
        }
        else if (msg->getKind() == 1)
        {
            ReconnectionSimulationEvent rEv = ReconnectionSimulationEvent();
            rEv.ExecuteEvent();

            cancelAndDelete(msg);
        }
        else
        {
            cancelAndDelete(msg);
            endSimulation();
        }
    }
}
