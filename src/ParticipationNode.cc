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

    //initialize constant for sortition
    two_to_the_hashlen = boost::multiprecision::pow(BigFloat(2.0), (64*8)+1);
}


ParticipationNode::~ParticipationNode()
{

}


void ParticipationNode::initialize()
{
    if (sodium_init() < 0)
    {
        EV << "ERROR INITIALIZING SODIUM" << endl;
        return;
    }


    //init 100 online accounts
    for(int i = 0; i < 100; i++)
    {
        Account a;
        a.Money = 100000;  //microalgos
        crypto_vrf_keypair(a.VRFKeys.VRFPubKey, a.VRFKeys.VRFPrivKey);
        OnlineAccounts.push_back(a);

//        EV << i << " ------ " << std::hex << a.VRFKeys.VRFPubKey << endl;
//        EV << i << " ------ " << std::hex << a.VRFKeys.VRFPrivKey << endl;
    }
}


void ParticipationNode::DeriveSeed(unsigned char* SeedHash, unsigned char* SeedProof, Account& a, unsigned int period, unsigned int step, unsigned int round)
{
    unsigned char* PrevSeed = Ledger.SeedLookup(round - SeedLookback);
    unsigned char alpha[64];

    //compute seed proof and preliminary hash
    if (period == 0)
    {
//        VRFOutput VRFOutSeed = RunVRF(a, PrevSeed, 32);
//        SeedHash = VRFOutSeed.VRFHash;
//        SeedProof = VRFOutSeed.VRFProof;
//
//        std::string VRFConcatAddress = std::string(&VRFOutSeed.VRFHash[0], &VRFOutSeed.VRFHash[64]) + std::itos(a.Address);
//        Hash_SHA512(alpha, (unsigned char*)(VRFConcatAddress.c_str()), VRFConcatAddress.size());
    }
    else
    {
//        SeedProof = {0};
//        Hash_SHA512(alpha, PrevSeed, 32);
    }


    //compute actual seed
    if (round % SeedLookback*SeedRefreshInterval < SeedLookback)
    {
//        std::string alphaConcatDigestLookup = std::string(&alpha[0], &alpha[64]) + std::string(Ledger.DigestLookup(round - SeedLookback*SeedRefreshInterval));
//        Hash_SHA512(SeedHash, (unsigned char*)(alphaConcatDigestLookup.c_str()), alphaConcatDigestlookup.size());
    }
//    else Hash_SHA512(SeedHash, alpha, 64);
}


bool ParticipationNode::VerifySeed()
{
     return true;
}


//TODO: llevar una cuenta de los mensajes recibidos y gossipeados en un determinado round
//solo puedo devolver mensaje UNA VEZ


VRFOutput ParticipationNode::RunVRF(Account& a, unsigned char* bytes, uint64_t bytesLen)
{
    VRFOutput cryptoDigest;

    crypto_vrf_prove(cryptoDigest.VRFProof, a.VRFKeys.VRFPrivKey, bytes, bytesLen);
    //retorna -1 si hay error decodificando la llave secreta, 0 si OK
    crypto_vrf_proof_to_hash(cryptoDigest.VRFHash, cryptoDigest.VRFProof);

    return cryptoDigest;
}


bool ParticipationNode::VerifyVRF(Account& a, unsigned char* bytes, uint64_t bytesLen, VRFOutput& HashAndProof)
{
    return crypto_vrf_verify(HashAndProof.VRFHash, a.VRFKeys.VRFPubKey, HashAndProof.VRFProof, bytes, bytesLen) == 0;
}


uint64_t ParticipationNode::sortition_binomial_cdf_walk(double n, double p, double ratio, uint64_t money)
{
  boost::math::binomial_distribution<double> dist(n, p);
  for (uint64_t j = 0; j < money; j++)
  {
      // Get the cdf
      double boundary = cdf(dist, j);

      // Found the correct boundary, break
      if (ratio <= boundary) return j;
  }
  return money;
}


BigInt ParticipationNode::byte_array_to_cpp_int(unsigned char* n, uint64_t len)
{
    BigInt i;
    uint32_t size = (uint32_t)len / sizeof(boost::multiprecision::limb_type);
    i.backend().resize(size, size);

    unsigned char reverse_n[64];
    std::reverse_copy(n, n + len, reverse_n);
    memcpy(i.backend().limbs(), reverse_n, len);
    i.backend().normalize();

    return i;
}


uint64_t ParticipationNode::Sortition(Account& a, uint64_t totalMoney, double expectedSize, VRFOutput& cryptoDigest)
{
    std::string seed = "00000000000000000000============";
    std::string role = "0";
    std::string m = seed + role;

    cryptoDigest = RunVRF(a, (unsigned char*)(m.c_str()), 65);


    double binomialN = double(a.Money);
    double binomialP = expectedSize / double(totalMoney);

    BigInt t = byte_array_to_cpp_int(cryptoDigest.VRFHash, 64);
    double ratio = (BigFloat(t) / two_to_the_hashlen).convert_to<double>();

    return sortition_binomial_cdf_walk(binomialN, binomialP, ratio, a.Money);
}


simpleBlock ParticipationNode::BlockProposal()
{
    VRFOutput VRFOut;
    for (int i = 0; i < 100; i++)
    {
        uint64_t SortitionOutput = Sortition(OnlineAccounts[i], 300000, 20, VRFOut);
        EV << "SortOutput : " << SortitionOutput << endl;
    }

    //TODO in this function:
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
