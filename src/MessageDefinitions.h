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

#ifndef MESSAGEDEFINITIONS_H_
#define MESSAGEDEFINITIONS_H_

#include <omnetpp/cmessage.h>
#include <omnetpp.h>
#include "DataTypeDefinitions.h"

#define REUSABLE_MSG_BUFFER_SIZE 1000

using namespace omnetpp;


class AlgorandMessage: public omnetpp::cMessage {
public:
    AlgorandMessage(){}
    AlgorandMessage(uint64_t r, short s, const char* str):step(s),round(r),cMessage(str), votes(0){}
    AlgorandMessage(AlgorandMessage* msg):round(msg->round), step(msg->step), votes(msg->votes),
            ProposerVRFCredentials(msg->ProposerVRFCredentials), ProposerAddress(msg->ProposerAddress),
            Payload(msg->Payload), cMessage(*msg){}
    virtual ~AlgorandMessage(){}

    virtual cMessage* dup() const override  {return new AlgorandMessage(*this);}

    inline void SetVotes(uint64_t j){votes=j;}
    void SetCredentials(VRFOutput& Cred, Address& I);
    inline void SetPayload(LedgerEntry& e){Payload = e;}


    //message recycling system
    static AlgorandMessage* MakeMessage(std::vector<AlgorandMessage*>& MsgBuffer, uint64_t round, short step, uint64_t votes,
            VRFOutput& SenderVRFCredentials, Address& SenderAddress, LedgerEntry& Block);
    static void RecycleMessage(cSimpleModule* callerModule, std::vector<AlgorandMessage*>& MsgBuffer, AlgorandMessage* msg);
    static AlgorandMessage* DuplicateMessage(std::vector<AlgorandMessage*>& MsgBuffer, AlgorandMessage* msg);

//protected:
    uint64_t round;
    short step;

    uint64_t votes;
    VRFOutput ProposerVRFCredentials;
    Address ProposerAddress;




    LedgerEntry Payload;
};


class ProposalMessage: public AlgorandMessage {
public:
    ProposalMessage();
    virtual ~ProposalMessage();
};


class FullBlockMessage: public AlgorandMessage {
public:
    FullBlockMessage();
    virtual ~FullBlockMessage();
};


class VoteMessage: public AlgorandMessage {
public:
    VoteMessage();
    virtual ~VoteMessage();

protected:
    uint64_t votes;
};


#endif /* MESSAGEDEFINITIONS_H_ */
