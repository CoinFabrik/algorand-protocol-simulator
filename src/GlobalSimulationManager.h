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
#include <unordered_set>
#include "SimulationEvent.h"

#ifndef DATATYPEDEFINITIONS_H_
    #include "DataTypeDefinitions.h"
#endif

#ifndef PARTICIPATIONNODE_H_
    #include "ParticipationNode.h"
#endif

using namespace omnetpp;

#define DATASIZE_DELAY_MULTIPLIER_TXN 1.0f
#define DATASIZE_DELAY_MULTIPLIER_VOTE 1.0f
#define DATASIZE_DELAY_MULTIPLIER_PROPOSAL 1.0f
#define DATASIZE_DELAY_MULTIPLIER_BUNDLE 1.0f

#define COMPUTATION_DELAY_DELTA_TXN_HANDLER 0.0f
#define COMPUTATION_DELAY_DELTA_VOTE_HANDLER 0.0f
#define COMPUTATION_DELAY_DELTA_PROPOSAL_HANDLER 0.0f
#define COMPUTATION_DELAY_DELTA_BUNDLE_HANDLER 0.0f


struct Relay
{
    //acts as an address
    uint64_t RelayID;

    //Actual connections. IDs are positions in the over-arching network definition
    std::vector<int> RelayConnections;          //Relay node IDs in network list
    std::vector<int> ParticipationConnections;  //Participation node IDs in network list


    //connection delay stuff
    std::vector<simtime_t> Relay_InConnectionDelays;
    std::vector<simtime_t> Relay_OutConnectionDelays;

    std::vector<simtime_t> PartNode_InConnectionDelays;
    std::vector<simtime_t> PartNode_OutConnectionDelays;


    bool RelayHasCustomCode = false;
};




class NetworkDefinition
{
public:
    std::vector<Relay> RelayNodes;
    std::vector<ParticipationNode*> ParticipationNodes;

    NetworkDefinition(){}
    void InitNetwork(int nPartNodes, int nRelayNodes, std::vector<std::vector<int>>& PartNodeConnections, std::vector<std::vector<int>>& RelayNodeConnections);
};




class GlobalSimulationManager : public cSimpleModule{
public:
    GlobalSimulationManager();
    virtual ~GlobalSimulationManager(){}
    void initialize();

    //singleton instance
    static GlobalSimulationManager* SimManager;


    //simulation parameter constants
//    const uint64_t TotalSupply = 10000000000;
    inline uint64_t getNumberOfPartNodes() const{return Network.ParticipationNodes.size();}
    inline uint64_t getNumberOfRelayNodes() const{return Network.RelayNodes.size();}

    uint64_t NumberOfAccounts;
    uint64_t NumberOfOnlineAccounts;

    uint64_t TotalMicroalgos;
    uint64_t TotalStakedMicroalgos;
    uint64_t Lookback_TotalStakedMicroalgos;


    //chronological time measuring
    std::chrono::time_point<std::chrono::high_resolution_clock> StartTime;
    std::chrono::duration<double> GetCurrentChronoTime(){return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - StartTime); }


    //represents "canonical" data
    Ledger Ledger;
    boost::unordered_map<Address, BalanceRecord> BalanceMap;


    //Network stuff
    NetworkDefinition Network;
    void AddParticipationNode();


    void PropagateMessageThroughNetwork(std::vector<int>& PendingRelayNodeVector, int SenderPartNode, void* msg, MsgType type);


    //scenario load and run data and functions
    void LoadContextFromFiles(std::string& NetworkDef_filename, std::string& BalanceDef_filename, std::string& CF_filename);
    void LoadEventsFromEF(std::string& filename);

    void AddAccountRecord(uint64_t address, uint64_t balance, bool status, std::vector<int>& patnodes);


    //simulation events to be executed
    std::vector<class SimulationEvent*> SimEventQueue;
    void handleMessage(cMessage* m);
    void ScheduleTimedSimulationEvents();


    //how many nodes are running round n
//    std::unordered_map<uint64_t, uint64_t> NodesInRound;


    //cached observed structures (improves memory allocation)
//    std::list<std::shared_ptr<Transaction>> LivingCirculatingTxns;




    //live circulating votes by round
    boost::unordered_map<uint64_t, std::unordered_map<uint64_t, Vote>> ObservedCirculatingVotes;
    Vote* AddCirculatingVote(Vote* vt);

    boost::unordered_map<uint64_t, std::unordered_map<uint64_t, ProposalPayload>> ObservedCirculatingProposals;
    ProposalPayload* AddCirculatingProposal(ProposalPayload* pp);

//    boost::unordered_map<uint64_t, std::unordered_map<uint64_t, Transaction>> ObservedCirculatingTransactions;
//    Transaction* AddCirculatingTransaction(Transaction* txn);

//    std::unordered_map<uint64_t, std::unordered_map<uint64_t, Bundle>> ObservedCirculatingBundles;
//    Bundle* AddCirculatingBundle(Bundle* b);


    //Global view of the ledger
    uint64_t CurrentGlobalRound;
    struct Ledger GlobalLedger;
    void NodeStartedNewRound(ParticipationNode* caller, uint64_t RoundStarted);
    void UpdateBalanceMap(LedgerEntry* e);
};


#endif /* GLOBALSIMULATIONMANAGER_H_ */
