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


enum ConsensusType{TENTATIVE, FINAL};


class AlgorandMessage: public omnetpp::cMessage {
public:
    AlgorandMessage(){}
    virtual ~AlgorandMessage(){}

protected:
    short step;
    uint64_t round;
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

private:
    ConsensusType Consensus;
};


#endif /* MESSAGEDEFINITIONS_H_ */
