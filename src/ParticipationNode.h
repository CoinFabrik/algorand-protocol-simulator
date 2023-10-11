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

#ifndef PARTICIPATIONNODE_H_
#define PARTICIPATIONNODE_H_

/*
 *
 */

#define SIMULATE_VRF 0
#define GLOBAL_BALANCE_TRACKER 1
#define SIMPLIFIED_BLOCKS 0
#define KEEP_GLOBAL_LEDGER 1

//parameter defines
#define TXN_POOL_LIMIT 1000
#define PARTNODE_LEDGER_CACHE 1000

//logging defines
#define LOG_STEP_EVENTS 1
#define LOG_VOTES 0
#define LOG_GLOBAL_BLOCKS 0
#define LOG_TXN_COMMITMENT 1

#define COMPUTE_SEED 0


//OMNET includes
#include <omnetpp.h>
#include <iomanip>
#include "DataTypeDefinitions.h"

//C/C++ standard includes
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <deque>
#include <algorithm>
#include <random>
#include <unordered_map>

//sodium library includes
#if !SIMULATE_VRF
    #include <sodium.h>
    #include "sodium/crypto_vrf.h"
    #include <openssl/evp.h>
#endif

#if COMPUTE_SEED
#endif

//boost includes
#include <boost/math/distributions/binomial.hpp>
#include <boost/random.hpp>
#include <boost/unordered_map.hpp>

using namespace omnetpp;


//logging of step events. Output is | "S" | node index | round | period | step number | simulation time | chronological time |
#if LOG_STEP_EVENTS
    #define OUT_LOG_STEP_EVENT() EV <<"S " << getIndex() << " " << round << " " << period << " "<< int(step) << " " << std::setprecision(4) << simTime() << " " << GlobalSimulationManager::SimManager->GetCurrentChronoTime().count() << endl
#else
    #define OUT_LOG_STEP_EVENT() ;
#endif

//logging of vote events. Output is | "V" | node index | round | period | step | account address | vote value (H(B)) | weight | simulation time | chronological time |
#if LOG_VOTES
    #define OUT_LOG_VOTE_EVENT(addr, val, weight) EV <<"V " << getIndex() << " " << round << " " << period << " "<< int(step) << " " << addr << " " << val << " " << weight << " " << simTime() << " " << std::setprecision(4) << GlobalSimulationManager::SimManager->GetCurrentChronoTime().count() << endl
#else
    #define OUT_LOG_VOTE_EVENT(addr, val, weight) ;
#endif

//logging of vote events. Output is | "B" | node index | account address | round | period | step | vote value (H(B)) | simulation time | chronological time |
#if LOG_GLOBAL_BLOCKS
    #define OUT_LOG_BLOCK_COMMITTED_GLOBAL_EVENT(round, block) EV <<"B " << round << " " << block.LedgerEntryID << " " << block.ProposerAddress << " " << block.Txns.size() << " " << simTime() << " " << std::setprecision(4) << GlobalSimulationManager::SimManager->GetCurrentChronoTime().count() << endl
#else
    #define OUT_LOG_BLOCK_COMMITTED_GLOBAL_EVENT(round, block) ;
#endif

//logging of disconnection events. Output is | "D" | node1 index | node2 index | round | period | step | simulation time | chronological time |
#if LOG_DISCONNECTION
    #define OUT_LOG_DISCONNECTION_EVENT(node1, node2) EV <<"D " << node1 << " " << node2 << " " << round << " " << period << " "<< int(step) << " " << simTime() << " " << std::setprecision(4) << GlobalSimulationManager::SimManager->GetCurrentChronoTime().count() << endl
#else
    #define OUT_LOG_DISCONNECTION_EVENT(node1, node2) ;
#endif

#if LOG_TXN_COMMITMENT
    #define OUT_LOG_TXN_COMMITMENT_EVENT(id, sender_addr, receiver_addr, amount) EV <<"T " << id << " " << sender_addr << " " << receiver_addr << " " << amount << " " << simTime() << " " << std::setprecision(4) << GlobalSimulationManager::SimManager->GetCurrentChronoTime().count() << endl
#else
    #define OUT_LOG_TXN_COMMITMENT_EVENT(sender_addr, receiver_addr, amount) ;
#endif


class ParticipationNode: public cSimpleModule
{
public:
    int NodeID;
    int getIndex(){return NodeID;}

    //random 256 bit number generator
    boost::random::independent_bits_engine<boost::random::mt19937, 256, uint256_t> generator;
    boost::random::independent_bits_engine<boost::random::mt19937, 256, uint256_t> BlockHashGenerator;

    ParticipationNode();
    ~ParticipationNode();
    void initialize();


    //balance tracking stuff
    void UpdateBalances();

#if GLOBAL_BALANCE_TRACKER
    boost::unordered_map<Address, BalanceRecord> BalanceMapLocalDivergence;
#endif


    //message handling (used as a "main" for timeouts)
    void handleMessage(cMessage* m);
    cMessage* FastResyncEvent;
    cMessage* TimeoutEvent;
    cMessage* TxnReceptionEvent;
    cMessage* VoteReceptionEvent;
    cMessage* ProposalReceptionEvent;
    cMessage* BundleReceptionEvent;
    SimTime startTime;


    //FSM variables
    uint64_t round;
    uint64_t period;

    uint8_t step;
    uint8_t lastConcludingStep;

    ProposalValue pinnedValue;


    uint64_t TotalStakedAlgos();
    std::vector<Account> offlineAccounts;
    std::vector<Account> onlineAccounts;
    Ledger Ledger;
    void AddGenesisBlock();




    //broadcasting functions
    void Broadcast(void* data, MsgType type);
    void Broadcast(Vote& vote);
    void Broadcast(ProposalPayload& proposal);
    void Broadcast(Bundle& bundle);

    //helper functions for special values
    ProposalValue Sigma();
    ProposalValue Mu();


    //helper functions for round control
    void StartNewRound();
    void StartNewPeriod(uint64_t FinishedPeriod);
    void GarbageCollectStateForNewRound();
    void GarbageCollectStateForNewPeriod(uint64_t NewPeriod);


    //round step functions
    LedgerEntry BlockAssembly();
    void DeriveSeed(stSeedAndProof& SeedAndProof, Account& a, unsigned int period, unsigned int round);
    void SetAddressDependantBlockData(LedgerEntry& e, Account* a);
    void BlockProposal();
    uint64_t ComputeBlockHash(LedgerEntry& e);

    void SoftVote();
    uint256_t ComputeLowestCredValue(VRFOutput& Credential, uint64_t weight);

#if SIMPLIFIED_BLOCKS
    void ConfirmBlock(uint256_t fakeEntry);
#else
    void ConfirmBlock(LedgerEntry& e);
#endif

    void NextVote();
    void FastRecovery();

    bool IsCommitable(ProposalValue& v);


    //cached full proposals
    boost::unordered_map<uint64_t, ProposalPayload*> CachedFullProposals[3];
    ProposalValue* WaitingForProposal;

    //bundle helper functions and data structures
    //a hash table, the entry tuple is a block's hash value
    boost::unordered_map<uint64_t, Bundle[256]> ActiveBundles[3];
    boost::unordered_map<uint64_t, Bundle[256]> FinishedBundles[3];
    Bundle* FreshestBundle;
    Bundle* SigmaBundle;  //bundle of the sigma value
    ProposalValue MuValue;

    //cache of received fast recovery votes for easy resynchronization attempts
    std::vector<Vote*> FastRecoveryVotes[3];


    inline uint8_t GetPrevPeriodSlot(){return CurrentPeriodSlot == 0 ? 2 : CurrentPeriodSlot-1; }
    inline uint8_t GetNextPeriodSlot(){return (CurrentPeriodSlot+1) % 3; }
    uint8_t CurrentPeriodSlot = 0;
    //address to vote map, per step. A maximum of two votes are permitted (one equivocation)
    //at most I keep 3 periods at all times (one forward, one curent, one backward)
    boost::unordered_map<Address, Vote*[2]> AddressToVoteMap[256][3];
//    //Equivocation vote handling stuff
//    struct EquivocationData
//    {
//        Bundle* CachedClosestBundle[255];  //cached closest bundle per step (current round and period)
//        //std::vector<EquivocationVote> ReceivedEqVotes[255];
//        //uint64_t combinedEqWeight;
//
//        void Clear()
//        {
//            for (int i = 0; i < 255; i++)
//            {
//                //CombinedEquivocationWeight[i] = 0;
//                CachedClosestBundle[i] = nullptr;
//            }
//        }
//    }EqData;
//    uint64_t CombinedEquivocationWeight[2][255];

    //message reception handlers
    void HandleTransaction(Transaction* ReceivedTxn);
    void HandleVote(Vote* ReceivedVote);
    void HandleProposal(ProposalPayload* ReceivedPP);
    void HandleBundle(Bundle& ReceivedBundle);


    void ScheduleTxnHandling(float delay, Transaction* txn);
    void ScheduleVoteHandling(float delay, Vote* vt);
    void ScheduleProposalHandling(float delay, ProposalPayload* pp);
    void ScheduleBundleHandling(float delay, Bundle& b);

    struct TravelingTxn{SimTime ScheduleTime; Transaction* txn; };
    struct TravelingVote{SimTime ScheduleTime; Vote* vt; };
    struct TravelingProposal{SimTime ScheduleTime; ProposalPayload* pp; };
    struct TravelingBundle{SimTime ScheduleTime; Transaction* b; };
//    std::vector<TravelingTxn> TravelingTxnQueue;
//    std::vector<TravelingVote> TravelingVoteQueue;
//    std::vector<TravelingProposal> TravelingProposalQueue;
//    std::vector<TravelingBundle> TravelingBundleQueue;
    std::vector<Transaction*> TravelingTxnQueue;
    std::vector<Vote*> TravelingVoteQueue;
    std::vector<ProposalPayload*> TravelingProposalQueue;
    std::vector<Bundle> TravelingBundleQueue;


    void ResynchronizationAttempt();


    //sortition functions
#if SIMULATE_VRF
    VRFOutput SimulateVRF();
#else
    VRFOutput RunVRF(Account& a, unsigned char* bytes, uint64_t bytesLen);
    bool VerifyVRF(Account& a, unsigned char* bytes, uint64_t bytesLen, VRFOutput& HashAndProof);
#endif
    uint64_t sortition_binomial_cdf_walk(double n, double p, double ratio, uint64_t money);
    uint64_t Sortition(Account& a, double expectedSize, VRFOutput& cryptoDigest, uint64_t s_round, uint64_t s_period, short s_step);
    uint64_t VerifySortition();
    static BigFloat two_to_the_hashlen; //constant for sortition


    //transaction pool stuff
    std::vector<Transaction*> TransactionPool;  //limit to 75000? txHandler_test.go
    void SimulateTransactions();
    Transaction GenerateRandomTransaction();
    Transaction GenerateKeyregTransaction(Account& a);


    //verification stuff
    bool VerifyTransaction(Transaction* txn);
    bool VerifyProposalPayload(ProposalPayload& pp);
    bool VerifyVote(Vote* vt);
    bool VerifyBundle(Bundle& b);


    //direct network connections (relays I see)
    std::vector<int> RelayConnections;
};


#endif /* PARTICIPATIONNODE_H_ */
