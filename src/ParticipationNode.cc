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
    two_to_the_hashlen = boost::multiprecision::pow(BigFloat(2.0), 64*8);
}


ParticipationNode::~ParticipationNode()
{

}


void ParticipationNode::initialize()
{
    //initialize sodium library
    if (sodium_init() < 0)
    {
        EV << "Error initializing sodium. Finishing simulation..." << endl;
        finish();
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

    AddGenesisBlock();
    InitBalanceTracker();
}


void ParticipationNode::AddGenesisBlock()
{
    LedgerEntry e;
    e.SeedAndProof.Seed = "hgyurteydhsjaskeudiaoapdlfkruiu9";
    Ledger.Entries.push_back(e);
    currentRound = 1;
}


void ParticipationNode::InitBalanceTracker()
{

}




void ParticipationNode::DeriveSeed(stSeedAndProof& SeedAndProof, Account& a, unsigned int period, unsigned int round)
{
    //unsigned char* PrevSeed = Ledger.SeedLookup(round - SeedLookback);
    //SEED LOOKUP NO IMPLEMENTADA! Por ahora:
    unsigned char* PrevSeed = new unsigned char[32];

    unsigned char alpha[crypto_hash_sha256_BYTES];

    //compute seed proof and preliminary hash
    if (period == 0)
    {
        VRFOutput VRFOutSeed = RunVRF(a, PrevSeed, 32);
        std::memcpy(&SeedAndProof.Seed, &VRFOutSeed.VRFHash[0], 32); //VER. Por ahora, me quedo con los 32 bytes menos significativos
        std::memcpy(&SeedAndProof.Proof, &VRFOutSeed.VRFProof[0], 80);

        std::string VRFConcatAddress = std::string(&VRFOutSeed.VRFHash[0], &VRFOutSeed.VRFHash[64]); //+ std::to_string(a.AccountAddress);
        Hash_SHA256(alpha, (unsigned char*)(VRFConcatAddress.c_str()), VRFConcatAddress.size());
    }
    else
    {
        ///SeedAndProof.Proof = {0};
        Hash_SHA256(alpha, PrevSeed, 32);
    }

    //compute actual seed
    if (round % SeedLookback*SeedRefreshInterval < SeedLookback)
    {
        //std::string alphaConcatDigestLookup = std::string((char*)(alpha)) + std::string((char*)(Ledger.DigestLookup(round - SeedLookback*SeedRefreshInterval)));
        //DIGEST LOOKUP NO IMPLEMENTADA! Por ahora:
        std::string alphaConcatDigestLookup = std::string((char*)(alpha)) + std::string("00000000000000000000000000000000");

        Hash_SHA256((unsigned char*)(SeedAndProof.Seed.c_str()), (unsigned char*)(alphaConcatDigestLookup.c_str()), alphaConcatDigestLookup.size());
    }
    else Hash_SHA256((unsigned char*)(SeedAndProof.Seed.c_str()), alpha, 32);




    delete[] PrevSeed;
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


uint64_t ParticipationNode::Sortition(Account& a, uint64_t totalMoney, double expectedSize, VRFOutput& cryptoDigest, short Step)
{
    std::string seed = "00000000000000000000============";
    std::string role = std::to_string(Step);
    std::string m = seed + role;

    cryptoDigest = RunVRF(a, (unsigned char*)(m.c_str()), m.length());


    double binomialN = double(a.Money);
    double binomialP = expectedSize / double(totalMoney);

    BigInt t = byte_array_to_cpp_int(cryptoDigest.VRFHash, 64);
    double ratio = (BigFloat(t) / two_to_the_hashlen).convert_to<double>();

    return sortition_binomial_cdf_walk(binomialN, binomialP, ratio, a.Money);
}


LedgerEntry* ParticipationNode::MakeBlock()
{

    return nullptr;
}


uint64_t ParticipationNode::TotalStakedAlgos()
{
    return 30000000;
}


simpleBlock ParticipationNode::BlockProposal()
{
//    LedgerEntry* Block = MakeBlock();
    simpleBlock BP_SelectedBlock = (rand() % 100);




    uint64_t TotalStake = TotalStakedAlgos();
    BigInt LowestHash = BigInt(two_to_the_hashlen);
    for (Account& a : OnlineAccounts)
    {
        VRFOutput VRFOut;
        uint64_t j = Sortition(a, TotalStake, CommitteeSize(0), VRFOut, 0);


//        si la cuenta fue seleccionada al menos una vez
//        unsigned char Hash_str[crypto_hash_sha512_BYTES] = {0};
//        while (j > 0)
//        {
//            std::string VRFHashConcatJ = std::string(&VRFOut.VRFHash[0], &VRFOut.VRFHash[64]) + std::to_string(j);
//            Hash_SHA512(Hash_str, (unsigned char*)(VRFHashConcatJ.c_str()), VRFHashConcatJ.length());
//            BigInt Hash = byte_array_to_cpp_int(Hash_str, crypto_hash_sha512_BYTES);
//
//            if (Hash < LowestHash)
//                LowestHash = Hash;
//
//            j--;
//        }


//        if (j > 0)
//        {
//            //divulgar el bloque
//            AlgorandMessage* msg = new AlgorandMessage(currentRound, 0, std::to_string(BP_SelectedBlock).c_str());
//            msg->SetCredentials(VRFOut, a.AccountAddress);
//            Gossip(msg);
//
//            EV << j << endl;
//        }


        if (j > 0)
        {
            BigInt Hash = byte_array_to_cpp_int(VRFOut.VRFHash, 64);
            if (Hash < LowestHash) LowestHash = Hash;
        }
    }

    AlgorandMessage* msg = new AlgorandMessage(currentRound, 0, std::to_string(BP_SelectedBlock).c_str());
    //msg->SetCredentials(VRFOut, a.AccountAddress);
    Gossip(msg);
    cancelAndDelete(msg);



    //share message
//    BP_SelectedBlock = ProcessMessage(msg);
    auto BP_OGBlockCache = BP_SelectedBlock;
//    Gossip(msg);
//
//    cancelAndDelete(msg);


    auto startTime = simTime();
    while (simTime() < startTime + BlockProposalDelayTime)
    {
        AlgorandMessage* newMsg = (AlgorandMessage*)(receive(BlockProposalDelayTime));
        if (newMsg)
        {
            //Rudimentary check for validity. TODO: check other properties (eg. credentials)
            if (newMsg->round == currentRound && newMsg->step == 0)
            {
                auto msgData = ProcessMessage(newMsg);
                if (msgData < BP_SelectedBlock)
                    BP_SelectedBlock = msgData;
            }

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


void ParticipationNode::Gossip(AlgorandMessage* m)
{
    for (int i = 0; i < gateSize("gate"); i++)
    {
        send(m->dup(), "gate$o", i);
    }
}


simpleBlock ParticipationNode::ProcessMessage(AlgorandMessage* msg)
{
    //TODO: process different kinds of messages
    //TODO: return a tuple with all relevant information

    std::stringstream stream(msg->getName());
    int x = 0;
    stream >> x;

    return x;
}


simpleBlock ParticipationNode::CountVotes(short step)
{
    std::map<simpleBlock, unsigned int> counts;
    auto startTime = simTime();

    while(true)
    {
        AlgorandMessage* m = (AlgorandMessage*)(receive(SoftVoteDelayTime));
        //Gossip(m);

        if (!m)
        {
            if(simTime() > startTime + SoftVoteDelayTime) return TIMEOUT;
        }
        else
        {
            simpleBlock value = ProcessMessage(m);

            counts[value] += m->votes;
            //counts[value] += 1;

            cancelAndDelete(m);

            //if (counts[value] >= CommitteeThreshold(step)) return value;
            //if (counts[value] >= 1) return value;
            if (counts[value] >= 10) return value;
        }
    }
}


void ParticipationNode::CommitteeVote(const simpleBlock& hblock, short step)
{
    for (Account& a : OnlineAccounts)
    {
        VRFOutput VRFOut;
        uint64_t j = Sortition(a, TotalStakedAlgos(), CommitteeSize(1), VRFOut, 1);

        AlgorandMessage* m = new AlgorandMessage(currentRound, step, std::to_string(hblock).c_str());
        m->SetVotes(j);
        Gossip(m);
        cancelAndDelete(m);
    }
}


simpleBlock ParticipationNode::SoftVote(const simpleBlock& hblock)
{
   //If proposal is NULL, do not vote!
    if (hblock != EMPTY_HASH)
        CommitteeVote(hblock, 1);

    simpleBlock hblock1 = CountVotes(1);

     if (hblock1 == TIMEOUT) return EMPTY_HASH;
     else return hblock1;
}


simpleBlock ParticipationNode::CertifyVote(const simpleBlock& hblock)
{
    //TODO: implementar period (next vote, ver que es late, down y redo)
    //TODO: implementar common coin solution (? reserchear si es algo que siguen usando en primer lugar)

    simpleBlock r = hblock;
    int step = 3;

    while(step < 255)
    {
        CommitteeVote(r, step);
        r = CountVotes(step);
        if (r == TIMEOUT) r = hblock;
        else if (r != EMPTY_HASH)
        {
            for (int s = step; s < step+3; s++) CommitteeVote(r, step);
            //if (step == 3) CommitteeVote(r);
            return r;
        }
        step++;


        CommitteeVote(r, step);
        r = CountVotes(step);
        if (r == TIMEOUT) r = EMPTY_HASH;
        else if (r == EMPTY_HASH)
        {
            for (int s = step; s < step+3; s++) CommitteeVote(r, step);
            return r;
        }
        step++;


        //COMMON COIN ACA. POR AHORA VOTO COMUN PARA BENCHMARKING
        CommitteeVote(r, step);
        //step++;   en el paper esta esta linea, pero creo que en la realidad no avanza el step aca
    }


    //TODO: rutina de recovery
    //por ahora, retornar el empty_hash. Es posible que esto sea lo que hace en REDO de todas formas
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
