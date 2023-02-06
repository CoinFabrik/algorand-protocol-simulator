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

    ReusableMessages.reserve(REUSABLE_MSG_BUFFER_SIZE);
}


ParticipationNode::~ParticipationNode()
{
    for (auto& m : ReusableMessages) cancelAndDelete(m);
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
    unsigned char* PrevSeed = nullptr; //uchar_ptr(Ledger.SeedLookup(currentRound - SeedLookback));
    unsigned char alpha[crypto_hash_sha256_BYTES];

    //compute seed proof and preliminary hash
    if (period == 0)
    {
        VRFOutput VRFOutSeed = RunVRF(a, PrevSeed, 32);
        std::memcpy(&SeedAndProof.Seed, &VRFOutSeed.VRFHash[0], VRFOutSeed.VRFHash.length()); //VER. Por ahora, me quedo con los 32 bytes menos significativos
        std::memcpy(&SeedAndProof.Proof, &VRFOutSeed.VRFProof[0], VRFOutSeed.VRFProof.length());

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

    crypto_vrf_prove(uchar_ptr(cryptoDigest.VRFProof), a.VRFKeys.VRFPrivKey, bytes, bytesLen);
    //retorna -1 si hay error decodificando la llave secreta, 0 si OK
    crypto_vrf_proof_to_hash(uchar_ptr(cryptoDigest.VRFHash), uchar_ptr(cryptoDigest.VRFProof));

    return cryptoDigest;
}


bool ParticipationNode::VerifyVRF(Account& a, unsigned char* bytes, uint64_t bytesLen, VRFOutput& HashAndProof)
{
    return crypto_vrf_verify(uchar_ptr(HashAndProof.VRFHash), a.VRFKeys.VRFPubKey, uchar_ptr(HashAndProof.VRFProof), bytes, bytesLen) == 0;
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

    BigInt t = byte_array_to_cpp_int(uchar_ptr(cryptoDigest.VRFHash), 64);
    double ratio = (BigFloat(t) / two_to_the_hashlen).convert_to<double>();

    return sortition_binomial_cdf_walk(binomialN, binomialP, ratio, a.Money);
}


LedgerEntry ParticipationNode::BlockAssembly()
{
    LedgerEntry e;
    e.PlaceholderID = (rand() % 100)+1;
    return e;
}


uint64_t ParticipationNode::TotalStakedAlgos()
{
    return 30000000;
}


LedgerEntry ParticipationNode::BlockProposal(LedgerEntry& Block)
{
    uint64_t TotalStake = TotalStakedAlgos();
    BigInt LowestHash = BigInt(two_to_the_hashlen);
    LedgerEntry SelectedBlock = Block;

    Account* ChosenAccount = nullptr;
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
            BigInt Hash = byte_array_to_cpp_int(uchar_ptr(VRFOut.VRFHash), 64);
            if (Hash < LowestHash)
            {
                LowestHash = Hash;
                SelectedBlock.ProposerCredentials = VRFOut;
                ChosenAccount = &a;
            }
        }
    }


    AlgorandMessage* msg = AlgorandMessage::MakeMessage(ReusableMessages, currentRound, 0, 0, SelectedBlock.ProposerCredentials, ChosenAccount->AccountAddress, SelectedBlock);
    Gossip(msg);
    AlgorandMessage::RecycleMessage(this, ReusableMessages, msg);


    auto startTime = simTime();
    while (simTime() < startTime + BlockProposalDelayTime)
    {
        AlgorandMessage* newMsg = (AlgorandMessage*)(receive(BlockProposalDelayTime));
        if (newMsg)
        {
            //Rudimentary check for validity. TODO: check other properties (eg. credentials)
            if (newMsg->round == currentRound && newMsg->step == 0)
            {
                auto msgData = newMsg->Payload.PlaceholderID;
                if (msgData < SelectedBlock.PlaceholderID)
                    SelectedBlock = newMsg->Payload;
            }

            AlgorandMessage::RecycleMessage(this, ReusableMessages, newMsg);
        }


    }

    //solo para benchmark por ahora, agrego el recibimiento del bloque completo
//    if (BP_OGBlockCache == BP_SelectedBlock) Gossip(new cMessage(str.c_str()));
//    while (simTime() < FullBlockDelayTime)
//    {
//        receive(FullBlockDelayTime);
//        break;
//    }


    return SelectedBlock;
}


void ParticipationNode::Gossip(AlgorandMessage* m)
{
    for (int i = 0; i < gateSize("gate"); i++)
    {
        send(AlgorandMessage::DuplicateMessage(ReusableMessages, m), "gate$o", i);
    }
}


int ParticipationNode::ProcessMessage(AlgorandMessage* msg)
{
    //TODO: process different kinds of messages
    //TODO: return a tuple with all relevant information

    return msg->Payload.PlaceholderID;
}


LedgerEntry ParticipationNode::CountVotes(short step, uint64_t localValue, uint64_t localVotes)
{
    std::map<int, unsigned int> counts;
    auto startTime = simTime();

    counts[localValue] += localVotes;

    while(true)
    {
        AlgorandMessage* m = (AlgorandMessage*)(receive(SoftVoteDelayTime));
        //Gossip(m);

        if (!m)
        {
            if(simTime() > startTime + SoftVoteDelayTime) return LedgerEntry(0); //TIMEOUT;
        }
        else
        {
            int value = m->Payload.PlaceholderID;
            auto le = m->Payload;

            //validacion rudimentaria. Aca validaria el voto entero?
            if (m->round == currentRound && m->step == step)
                counts[value] += m->votes;

            AlgorandMessage::RecycleMessage(this, ReusableMessages, m);

            if (counts[value] >= CommitteeThreshold(step)) return le;
            //if (counts[value] >= 10) return le;
        }
    }
}


uint64_t ParticipationNode::CommitteeVote(LedgerEntry& hblock, short step)
{
    uint64_t localVotes = 0;
    for (Account& a : OnlineAccounts)
    {
        VRFOutput VRFOut;
        uint64_t j = Sortition(a, TotalStakedAlgos(), CommitteeSize(step), VRFOut, step);
        //uint64_t j = Sortition(a, TotalStakedAlgos(), CommitteeSize(0), VRFOut, step);

        localVotes+= j;

        if (j > 0)
        {
            AlgorandMessage* m = AlgorandMessage::MakeMessage(ReusableMessages, currentRound, step, j, VRFOut, a.AccountAddress, hblock);
            Gossip(m);
            AlgorandMessage::RecycleMessage(this, ReusableMessages, m);
        }
    }
    return localVotes;
}


LedgerEntry ParticipationNode::SoftVote(LedgerEntry& hblock)
{
    uint64_t localVotes = 0;

   //If proposal is NULL, do not vote!
    if (hblock.PlaceholderID != EMPTY_HASH)
        localVotes = CommitteeVote(hblock, 1);

    LedgerEntry hblock1 = CountVotes(1, hblock.PlaceholderID, localVotes);

     if (hblock1.PlaceholderID == TIMEOUT) return hblock1;
     else return hblock1;
}


LedgerEntry ParticipationNode::CertifyVote(LedgerEntry& hblock)
{
    //TODO: implementar period (next vote, ver que es late, down y redo)
    //TODO: implementar common coin solution (? reserchear si es algo que siguen usando en primer lugar)

    LedgerEntry r = hblock;
    int step = 3;

    while(step < 255)
    {
        uint64_t localVotes = CommitteeVote(r, step);
        r = CountVotes(step, r.PlaceholderID, localVotes);
        if (r.PlaceholderID == TIMEOUT) r = hblock;
        else if (r.PlaceholderID != EMPTY_HASH)
        {
            for (int s = step; s < step+3; s++) CommitteeVote(r, step);
            return r;
            //TODO: incorporate logic for nextvote
        }
        step++;


        localVotes = CommitteeVote(r, step);
        r = CountVotes(step, r.PlaceholderID, localVotes);
        if (r.PlaceholderID == TIMEOUT) r = LedgerEntry(0);
        else if (r.PlaceholderID == EMPTY_HASH)
        {
            for (int s = step; s < step+3; s++) CommitteeVote(r, step);
            return r;
        }
        step++;


        //COMMON COIN ACA?
        //CommitteeVote(r, step);
        //step++;   en el paper esta esta linea, pero creo que en la realidad no avanza el step aca
    }


    //TODO: rutina de recovery
    //por ahora, retornar el empty_hash. Es posible que esto sea lo que hace en REDO de todas formas


    //down vote (step 255) must vote for empty block
    r = LedgerEntry(0);
    CommitteeVote(r, step);
    return r;
}


void ParticipationNode::ConfirmBlock(const LedgerEntry& hblock)
{
    //TODO: tipo de consenso. Por ahora, confirma el bloque que recibe
    //CountVotes(); //conteo de votos para ver

    Ledger.Entries.push_back(hblock);
}


void ParticipationNode::activity()
{
    //por ahora, para generar variabilidad de mensajes
    srand(this->getId());

    while(true)
    {
        LedgerEntry localBlockVal = BlockAssembly();
        EV << "Node " << this->getId() << " finished BA stage\n";

        localBlockVal = BlockProposal(localBlockVal);
        EV << "Node " << this->getId() << " finished BP stage\n";

        localBlockVal = SoftVote(localBlockVal);
        EV << "Node " << this->getId() << " finished SV stage\n";

        localBlockVal = CertifyVote(localBlockVal);
        EV << "Node " << this->getId() << " finished CV stage\n";

        ConfirmBlock(localBlockVal);
        EV_DETAIL << "Block "<< localBlockVal.PlaceholderID <<" confirmed by node " << this->getId() << "\n";

        currentRound++;
        EV_DETAIL << "ROUND " << currentRound-1 << " FINISHED SUCCESFULY BY NODE "<< this->getId() << "AT SIMTIME " << simTime() << "\n";
    }
}


void ParticipationNode::finish()
{

}
