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

#define TOTAL_NODES 200
#define START_MONEY 100000
#define TOTAL_ACCOUNTS 10

//OMNET includes
#include <omnetpp.h>
//#include <omnetpp/cqueue.h>
//#include "MessageDefinitions.h"
#include "DataTypeDefinitions.h"

//C/C++ standard includes
#include <iostream>
#include <string.h>
#include <stdlib.h>
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


/**********************************************************************************/
//PROTOCOL PARAMETERS (from abft.pdf in specs)
unsigned int SeedLookback = 2;
unsigned int SeedRefreshInterval = 80;
unsigned int BalanceLookback = 2*SeedLookback*SeedRefreshInterval;

//time parameters (in seconds). TODO: darles nombres mas descriptivos
float Lambda = 2.f;
float Lambda0 = 1.7f;
float LambdaF = 300.f; //5 min
float UppercaseLambda = 17.f;

inline float FilterTimeout(unsigned int p){return 2.f * (p==0? Lambda0 : Lambda);}

//useful constants
ProposalValue EMPTY_PROPOSAL_VALUE;
/**********************************************************************************/


class ParticipationNode: public cSimpleModule
{
public:
    //random 256 bit number generator
    boost::random::independent_bits_engine<boost::random::mt19937, 256, uint256_t> generator;

    ParticipationNode();
    ~ParticipationNode();
    void initialize();

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
    // ProposalPayload pinnedPayload;


    uint64_t TotalStakedAlgos();
    void InitOnlineAccounts();
    std::vector<Account> onlineAccounts;
    Ledger Ledger;




    //proposal and vote sets
    std::vector<ProposalPayload> P;
    //std::vector<Vote> V;

    //boradcasting functions
    void Broadcast(Vote& v);
    void Broadcast(ProposalPayload& p);

    //helper functions for special values
    ProposalValue Sigma();
    ProposalValue Mu();


    //helper functions for round control
    void StartNewRound();
    void StartNewPeriod(uint64_t FinishedPeriod);
    void GarbageCollectStateForNewRound();
    void GarbageCollectStateForNewPeriod();


    //round step functions
    void BlockAssembly();
    void BlockProposal();

    void SoftVote();
    uint256_t ComputeLowestCredValue(VRFOutput& Credential, uint64_t weight);

    void ConfirmBlock(uint256_t fakeEntry);

    void NextVote();
    void FastRecovery();

    bool IsCommitable(ProposalValue& v);


    std::vector<Vote> ProposalVotes;

    //bundle helper functions and data structures
    //a hash table, the entry tuple is a pair of (block digest)
    std::unordered_map<uint256_t, Bundle[255]> ActiveBundles[2];
    std::unordered_map<uint256_t, Bundle[255]> FinishedBundles[2];
    Bundle* FreshestBundle;
    Bundle* SigmaBundle;
    ProposalValue MuValue;

    std::vector<Vote> FastRecoveryVotes[3];

    //std::vector<EquivocationVote> ReceivedEqVotes;
    //std::unordered_map<uint256_t, Vote>AddressToVoteMap;  //for repeated votes and equivocation votes


    //message reception handlers
    void HandleVote(Vote& ReceivedVote);
    void HandleProposalPayload(ProposalPayload& ReceivedPP);
    void HandleBundle(Bundle& ReceivedBundle);



    void ResynchronizationAttempt();



    //sortition functions
    VRFOutput SimulateVRF();
    VRFOutput RunVRF(Account& a, unsigned char* bytes, uint64_t bytesLen);
    bool VerifyVRF(Account& a, unsigned char* bytes, uint64_t bytesLen, VRFOutput& HashAndProof);
    uint64_t sortition_binomial_cdf_walk(double n, double p, double ratio, uint64_t money);
    uint64_t Sortition(Account& a, uint64_t totalMoney, double expectedSize, VRFOutput& cryptoDigest, short step);
    uint64_t VerifySortition();

    BigFloat two_to_the_hashlen; //constant for sortition
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
