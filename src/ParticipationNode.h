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

#include <omnetpp.h>
#include <omnetpp/cqueue.h>

#include <string.h>
#include <stdlib.h>

#include <sodium.h>
#include "sodium/crypto_vrf.h"
#include "DataTypeDefinitions.h"

#include <boost/math/distributions/binomial.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>

//#include <boost/multiprecision/cpp_int/serialize.hpp>
//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/binary_oarchive.hpp>
//#include <boost/iostreams/device/back_inserter.hpp>
//#include <boost/iostreams/device/array.hpp>
//#include <boost/iostreams/stream.hpp>

#include <iostream>


using namespace omnetpp;
typedef boost::multiprecision::cpp_int BigInt;
//typedef boost::multiprecision::number<boost::multiprecision::cpp_bin_float<64*10+1>> BigFloat;
typedef boost::multiprecision::number<boost::multiprecision::cpp_bin_float<2048>> BigFloat;


const simpleBlock EMPTY_HASH = 0;
const unsigned int TIMEOUT = UINT_MAX;


/*************************/
//GLOBAL NETWORK PARAMETERS
unsigned int BlockProposalDelayTime = 5;
unsigned int FullBlockDelayTime = 60;
unsigned int SoftVoteDelayTime = 20;
unsigned int CertVoteDelayTime = 10;

unsigned int SoftVoteThreshold = 2;
/*************************/


/*************************/
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
/*************************/






class ParticipationNode: public cSimpleModule
{

public:
    ParticipationNode();
    virtual ~ParticipationNode();

    void initialize();
    void activity();

    void finish();



    //node specific helper functions
    void Gossip(cMessage* m);

    BigFloat two_to_the_hashlen;
    VRFOutput RunVRF(Account& a, uint8_t* SeedAndRole);
    uint64_t sortition_binomial_cdf_walk(double n, double p, double ratio, uint64_t money);
    uint64_t Sortition(Account& a, uint64_t totalMoney, double expectedSize, VRFOutput& cryptoDigest);
    boost::multiprecision::cpp_int byte_array_to_cpp_int(unsigned char* n, uint64_t size);
    uint64_t VerifySortition();


    simpleBlock ProcessMessage(cMessage* msg);
    void CommitteeVote(const simpleBlock& hblock);
    simpleBlock CountVotes();

    //node specific main functions
    simpleBlock BlockProposal();
    simpleBlock SoftVote(const simpleBlock& hblock);
    simpleBlock CertifyVote(const simpleBlock& hblock);
    void ConfirmBlock(const simpleBlock& hblock);


protected:
    Ledger LocalLedgerCopy;
    uint64_t currentRound;


    std::vector<Account> OnlineAccounts;
    //cQueue UnprocessedMessages;
};


#endif /* PARTICIPATIONNODE_H_ */
