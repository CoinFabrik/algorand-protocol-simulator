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
#include <string.h>
//#include <omnetpp/csimplemodule.h>
#include <omnetpp/cqueue.h>
//#include <omnetpp/cmessage.h>
#include <stdlib.h>
#include <sodium.h>
//#include <boost/math/distributions/binomial.hpp>


using namespace omnetpp;


typedef unsigned int simpleBlock;
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


struct Account
{
    uint64_t Address;
    uint64_t money;
};


struct VRFOutput
{
    char VrfPrivkey[64];
    char VrfPubkey[32];
    char VRFProof[80];
    char VRFHash[64];
};


struct Ledger
{
    std::vector<unsigned int> blocks;
};


//uint64_t sortition_binomial_cdf_walk(double n, double p, double ratio, uint64_t money)
//{
//  boost::math::binomial_distribution<double> dist(n, p);
//  for (uint64_t j = 0; j < money; j++)
//  {
//      // Get the cdf
//      double boundary = cdf(dist, j);
//
//      // Found the correct boundary, break
//      if (ratio <= boundary) return j;
//  }
//  return money;
//}


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
//    uint64_t Sortition(uint64_t money, uint64_t totalMoney, double expectedSize, vrfOutput crypto.Digest);
    int VerifySortition();
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
