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

#include "ParticipationNode.h"

Define_Module(ParticipationNode);

ParticipationNode::ParticipationNode():cSimpleModule(4096)
{
    currentRound = 0;
}


ParticipationNode::~ParticipationNode()
{

}


void ParticipationNode::initialize()
{

    if (sodium_init() < 0)
    {
        return;
    }
}


//TODO: llevar una cuenta de los mensajes recibidos y gossipeados en un determinado round
//solo puedo devolver mensaje UNA VEZ

//uint64_t ParticipationNode::sortition(uint64_t money, uint64_t totalMoney, double expectedSize, vrfOutput crypto.Digest)
//{
//    double binomialN = double(money);
//}


simpleBlock ParticipationNode::BlockProposal()
{
    //TODO in this function:
    //implement sortition
    //implement sortition validation
    //implement block structures
    //implement 2 kinds of proposal messages (full block, brief version)
    //implement hash function for blocks


    simpleBlock BP_SelectedBlock = INT_MAX;


    //primero sortition
    //por ahora, siempre propone uno solo
    auto str = std::to_string(rand() % 100);
    cMessage* msg = new cMessage(str.c_str());


    //share message
    BP_SelectedBlock = ProcessMessage(msg);
    auto BP_OGBlockCache = BP_SelectedBlock;
    Gossip(msg);

    cancelAndDelete(msg);


    auto startTime = simTime();
    while (simTime() < startTime + BlockProposalDelayTime)
    {
        cMessage* newMsg = receive(BlockProposalDelayTime);
        if (newMsg)
        {
            //TODO: check validity

            auto msgData = ProcessMessage(newMsg);
            if (msgData < BP_SelectedBlock)
                BP_SelectedBlock = msgData;

            //Gossip(newMsg);

            cancelAndDelete(newMsg);
        }


    }

    //solo para benchmark por ahora, agrego el recibimiento del bloque completo
//    if (BP_OGBlockCache == BP_SelectedBlock) Gossip(new cMessage(str.c_str()));
//    while (simTime() < FullBlockDelayTime)
//    {
//        receive(FullBlockDelayTime);
//        break;
//    }


    return BP_SelectedBlock;
}


void ParticipationNode::Gossip(cMessage* m)
{
    for (int i = 0; i < gateSize("gate"); i++)
    {
        send(m->dup(), "gate$o", i);
    }
}


simpleBlock ParticipationNode::ProcessMessage(cMessage* msg)
{
    //TODO: process different kinds of messages
    //TODO: return a tuple with all relevant information

    std::stringstream stream(msg->getName());
    int x = 0;
    stream >> x;

    return x;
}


simpleBlock ParticipationNode::CountVotes()
{
    std::map<simpleBlock, unsigned int> counts;
    auto startTime = simTime();

    while(true)
    {
        cMessage* m = receive(SoftVoteDelayTime);
        //Gossip(m);

        if (!m)
        {
            if(simTime() > startTime + SoftVoteDelayTime) return TIMEOUT;
        }
        else
        {
            simpleBlock value = ProcessMessage(m);
            cancelAndDelete(m);

            counts[value] += 1;
            if (counts[value] >= SoftVoteThreshold) return value;
        }
    }
}


void ParticipationNode::CommitteeVote(const simpleBlock& hblock)
{
    //TODO: check if user is in committee using sortition

    cMessage* m = new cMessage(std::to_string(hblock).c_str());
    Gossip(m);
    cancelAndDelete(m);
}


simpleBlock ParticipationNode::SoftVote(const simpleBlock& hblock)
{
    CommitteeVote(hblock);
    simpleBlock hblock1 = CountVotes();

    if (hblock1 == TIMEOUT) CommitteeVote(EMPTY_HASH);
    else CommitteeVote(hblock1);

    hblock1 = CountVotes();
    if (hblock1 == TIMEOUT) return EMPTY_HASH;
    else return hblock1;
}


simpleBlock ParticipationNode::CertifyVote(const simpleBlock& hblock)
{
    //TODO: implementar nociones de final y tentative consensus
    //TODO: implementar common coin solution

    simpleBlock r = hblock;
    int step = 3;

    while(step < 255)
    {
        CommitteeVote(r);
        r = CountVotes();
        if (r == TIMEOUT) r = hblock;
        else if (r != EMPTY_HASH)
        {
            for (int s = step; s < step+3; s++) CommitteeVote(r);
            //if (step == 1) CommitteeVote(r);
            return r;
        }
        step++;


        CommitteeVote(r);
        r = CountVotes();
        if (r == TIMEOUT) r = EMPTY_HASH;
        else if (r == EMPTY_HASH)
        {
            for (int s = step; s < step+3; s++) CommitteeVote(r);
            return r;
        }
        step++;


        //COMMON COIN ACA. POR AHORA VOTO COMUN PARA BENCHMARKING
        CommitteeVote(r);
        step++;
    }


    //TODO: rutina de recovery
    //por ahora, retornar el empty_hash
    return EMPTY_HASH;
}


void ParticipationNode::ConfirmBlock(const simpleBlock& hblock)
{
    //TODO: tipo de consenso. Por ahora, confirma el bloque que recibe
    //CountVotes(); //conteo de votos para ver el tipo de consenso

    LocalLedgerCopy.blocks.push_back(hblock);
}


void ParticipationNode::activity()
{
    //por ahora, para generar variabilidad de mensajes
    srand(this->getId());

    while(true)
    {
        simpleBlock localBlockVal = BlockProposal();
        EV << "Node " << this->getId() << " finished BP stage\n";

        localBlockVal = SoftVote(localBlockVal);
        EV << "Node " << this->getId() << " finished SV stage\n";

        localBlockVal = CertifyVote(localBlockVal);
        EV << "Node " << this->getId() << " finished CV stage\n";

        ConfirmBlock(localBlockVal);
        EV << "Block "<< localBlockVal <<" confirmed by node " << this->getId() << "\n";

        currentRound++;
        EV << "ROUND " << currentRound-1 << " FINISHED SUCCESFULY BY NODE "<< this->getId() << "!\n";
    }
}


void ParticipationNode::finish()
{

}
