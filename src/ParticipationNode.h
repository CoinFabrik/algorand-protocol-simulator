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

#define SIMULATE_VRF 1
#define GLOBAL_BALANCE_TRACKER 1
#define SIMPLIFIED_BLOCKS 1
#define KEEP_GLOBAL_LEDGER 1


#define TOTAL_NODES 400
#define RELAYS 100
#define START_MONEY 1000000
#define TOTAL_ACCOUNTS 10

//logging defines
#define LOG_STEP_EVENTS 1


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
#include <map>

//sodium library includes
#include <sodium.h>
#include "sodium/crypto_vrf.h"

//boost includes
#include <boost/math/distributions/binomial.hpp>
#include <boost/random.hpp>

using namespace omnetpp;


//logging of step events. Output is | "S" | node index | round | period | step number | simulation time | chronological time|
#if LOG_STEP_EVENTS
    #define OUT_LOG_STEP_EVENT() EV <<"S " << getIndex() << " " << round << " " << period << " "<< int(step) << " " << simTime() << " " << std::setprecision(15) << GlobalSimulationManager::SimManager->GetCurrentChronoTime().count() << endl
#else
    #define OUT_LOG_STEP_EVENT() ;
#endif


class ParticipationNode: public cSimpleModule
{
public:

    //random 256 bit number generator
    boost::random::independent_bits_engine<boost::random::mt19937, 256, uint256_t> generator;
    boost::random::independent_bits_engine<boost::random::mt19937, 256, uint256_t> BlockHashGenerator;

    ParticipationNode();
    ~ParticipationNode();
    void initialize();


    //balance tracking stuff
    void UpdateBalances();

#if GLOBAL_BALANCE_TRACKER
    std::unordered_map<Address, BalanceRecord> BalanceMapLocalDivergence;
#endif


    //message handling (used as a "main" for timeouts)
    void handleMessage(cMessage* m);
    cMessage* FastResyncEvent;
    cMessage* TimeoutEvent;
    SimTime startTime;


    //FSM variables
    uint64_t round;
    uint64_t period;

    uint8_t step;
    uint8_t lastConcludingStep;

    ProposalValue pinnedValue;


    uint64_t TotalStakedAlgos();
    void InitOnlineAccounts();
    std::vector<Account> onlineAccounts;
    Ledger Ledger;




    //observed proposals set
    std::vector<ProposalPayload> P;

    //multi-purpose boradcasting function
    void Broadcast(void* data, MsgType type);

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
    void BlockProposal(LedgerEntry& e);
    uint256_t ComputeBlockHash(LedgerEntry& e);

    void SoftVote();
    uint256_t ComputeLowestCredValue(VRFOutput& Credential, uint64_t weight);

    void ConfirmBlock(uint256_t fakeEntry);

    void NextVote();
    void FastRecovery();

    bool IsCommitable(ProposalValue& v);


    //cached received proposal votes
    std::vector<Vote> ProposalVotes;
    std::unordered_map<uint256_t, LedgerEntry> CachedFullProposals[3];

    //bundle helper functions and data structures
    //a hash table, the entry tuple is a block's hash value
    std::unordered_map<uint256_t, Bundle[256]> ActiveBundles[3];
    std::unordered_map<uint256_t, Bundle[256]> FinishedBundles[3];
    Bundle* FreshestBundle;
    Bundle* SigmaBundle;  //bundle of the sigma value
    ProposalValue MuValue;

    //cache of received fast recovery votes for easy resynchronization attempts
    std::vector<Vote> FastRecoveryVotes[3];


    inline uint8_t GetPrevPeriodSlot(){return CurrentPeriodSlot == 0 ? 2 : CurrentPeriodSlot-1; }
    inline uint8_t GetNextPeriodSlot(){return (CurrentPeriodSlot+1) % 3; }
    uint8_t CurrentPeriodSlot = 0;
    //address to vote map, per step. A maximum of two votes are permitted (one equivocation)
    //at most I keep 3 periods at all times (one forward, one curent, one backward)
    std::map<Address, Vote[2]> AddressToVoteMap[256][3];
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
    void HandleTransaction(Transaction& ReceivedTxn);
    void HandleVote(Vote& ReceivedVote);
    void HandleProposal(ProposalPayload& ReceivedPP);
    void HandleBundle(Bundle& ReceivedBundle);


    void TEST_ScheduleVoteHandling(float delay, Vote& vt);
    std::vector<Vote> VoteQueue;




    void ResynchronizationAttempt();




    //sortition functions
    VRFOutput SimulateVRF();
    VRFOutput RunVRF(Account& a, unsigned char* bytes, uint64_t bytesLen);
    bool VerifyVRF(Account& a, unsigned char* bytes, uint64_t bytesLen, VRFOutput& HashAndProof);
    uint64_t sortition_binomial_cdf_walk(double n, double p, double ratio, uint64_t money);
    uint64_t Sortition(Account& a, uint64_t totalMoney, double expectedSize, VRFOutput& cryptoDigest, short step);
    uint64_t VerifySortition();

    BigFloat two_to_the_hashlen; //constant for sortition




    //transaction pool stuff
    std::vector<Transaction> TransactionPool;
    void SimulateTransactions();
    Transaction GenerateRandomTransaction();




    //verification stuff
    bool VerifyProposalPayload(LedgerEntry& e);
    bool VerifyVote(Vote& vt);
    bool VerifyBundle(Bundle& b);



    //TESTING
    //direct network connections (relays I see)
    std::vector<int> RelayConnections;
};






// class ParticipationNode: public cSimpleModule
// {

//     std::vector<LedgerEntry> P;   //observedProposals

//     std::vector<Vote> SoftVotes;  //observedSoftVotes       //ver: sorted by value?
//     std::vector<Vote> CertVotes;  //observedCertVotes
//     std::vector<Vote> RecoveryVotes;

//     std::vector<Vote> PeriodObservedVotes[256];


//     std::set<struct Proposal> ObservedProposals;


//     //node initialization functions
//     void AddGenesisBlock();
//     void InitBalanceTracker();


//     //signature functions
//     void Sign(Account& a, unsigned char* data, unsigned char* outSignedData);
//     uint64_t VerifySignature(unsigned char* SignedData, unsigned char* PK); //outputs the "weight" of signature


//     //multipurpose cryptographic hash. TODO: implement SHA512/256 (no esta en sodium?)
//     inline void Hash_SHA256(unsigned char* out, unsigned char* in, uint64_t len){crypto_hash_sha256(out, in, len);}  //TODO: switch for SHA512/256


//     //seed computation and verification
//     void DeriveSeed(stSeedAndProof& SeedAndProof, Account& a, unsigned int period, unsigned int round);
//     bool VerifySeed();


//     //main algorithm subroutines
//     uint64_t TotalStakedAlgos();

//     void ConfirmBlock(const LedgerEntry& hblock);


// protected:


//     std::vector<Account> OnlineAccounts;
//     Ledger Ledger;


#endif /* PARTICIPATIONNODE_H_ */
