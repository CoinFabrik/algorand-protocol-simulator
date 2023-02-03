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

//OMNET includes
#include <omnetpp.h>
#include <omnetpp/cqueue.h>
#include "MessageDefinitions.h"

//C/C++ standard includes
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <algorithm>

//sodium library includes
#include <sodium.h>
#include "sodium/crypto_vrf.h"

//boost includes
#include <boost/math/distributions/binomial.hpp>


using namespace omnetpp;


const simpleBlock EMPTY_HASH = 0;
const unsigned int TIMEOUT = UINT_MAX;


/*************************/
//GLOBAL NETWORK PARAMETERS
unsigned int BlockProposalDelayTime = 5;
unsigned int FullBlockDelayTime = 60;
unsigned int SoftVoteDelayTime = 200; //20
unsigned int CertVoteDelayTime = 10;

unsigned int SoftVoteThreshold = 2;
/*************************/


/**********************************************************************************/
//PROTOCOL PARAMETERS (from abft.pdf in specs)
unsigned int SeedLookback = 2;
unsigned int SeedRefreshInterval = 80;
unsigned int BalanceLookback = 2*SeedLookback*SeedRefreshInterval;

//time parameters (in seconds). TODO: darles nombres mas descriptivos
float Lambda = 2.f;
float Lambda0 = 1.7f;
float LambdaF = 5.f * 60.f; //5 min
float UppercaseLambda = 17.f;

inline float FilterTimeout(unsigned int p){return 2.f * (p==0? Lambda0 : Lambda);}
/**********************************************************************************/






class ParticipationNode: public cSimpleModule
{

public:
    ParticipationNode();
    virtual ~ParticipationNode();

    void initialize();
    void activity();

    void finish();


    //node initialization functions
    void AddGenesisBlock();
    void InitBalanceTracker();

    //broadcasting functions
    void Gossip(AlgorandMessage* m);


    //signature functions
    void Sign(Account& a, unsigned char* data, unsigned char* outSignedData);
    uint64_t VerifySignature(unsigned char* SignedData, unsigned char* PK); //outputs the "weight" of signature


    //multipurpose cryptographic hash. TODO: implement SHA512/256 (no esta en sodium?)
    inline void Hash_SHA256(unsigned char* out, unsigned char* in, uint64_t len){crypto_hash_sha256(out, in, len);}


    //seed computation and verification
    void DeriveSeed(stSeedAndProof& SeedAndProof, Account& a, unsigned int period, unsigned int round);
    bool VerifySeed();


    //sortition functions
    VRFOutput RunVRF(Account& a, unsigned char* bytes, uint64_t bytesLen);
    bool VerifyVRF(Account& a, unsigned char* bytes, uint64_t bytesLen, VRFOutput& HashAndProof);
    uint64_t sortition_binomial_cdf_walk(double n, double p, double ratio, uint64_t money);
    uint64_t Sortition(Account& a, uint64_t totalMoney, double expectedSize, VRFOutput& cryptoDigest, short Step);
    uint64_t VerifySortition();


    //main algorithm subroutines
    uint64_t TotalStakedAlgos();
    LedgerEntry* MakeBlock();
    simpleBlock ProcessMessage(AlgorandMessage* msg);
    void CommitteeVote(const simpleBlock& hblock, short step);
    simpleBlock CountVotes(short step);

    //main algorithm functions
    simpleBlock BlockProposal();
    simpleBlock SoftVote(const simpleBlock& hblock);
    simpleBlock CertifyVote(const simpleBlock& hblock);
    void ConfirmBlock(const simpleBlock& hblock);


protected:
    BigFloat two_to_the_hashlen; //constant for sortition


    Ledger LocalLedgerCopy;
    uint64_t currentRound;


    std::vector<Account> OnlineAccounts;
    //cQueue UnprocessedMessages;

    NewLedger Ledger;
};


#endif /* PARTICIPATIONNODE_H_ */
