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


ParticipationNode::ParticipationNode() : round(1), period(0), step(0), lastConcludingStep(0)//, pinnedVote(0)
{
//    uint64_t pinnedVote;




    //initialize constant for sortition
    two_to_the_hashlen = boost::multiprecision::pow(BigFloat(2.0), 64*8);

    //ReusableMessages.reserve(REUSABLE_MSG_BUFFER_SIZE);
    ReusableMessages.resize(REUSABLE_MSG_BUFFER_SIZE);
    for (int i = 0; i < ReusableMessages.size(); i++) ReusableMessages[i] = new AlgorandMessage();

    FastResyncEvent = new cMessage(nullptr, 253);
    TimeoutEvent = new cMessage(nullptr, 0);
}


ParticipationNode::~ParticipationNode()
{
    for (auto& m : ReusableMessages) cancelAndDelete(m);
    cancelAndDelete(FastResyncEvent);
    cancelAndDelete(TimeoutEvent);
}


void ParticipationNode::initialize()
{
    //Timeout starter self-event
    scheduleAfter(0, TimeoutEvent);

    //initialize sodium library
    if (sodium_init() < 0)
    {
        EV << "Error initializing sodium. Finishing simulation..." << endl;
        finish();
    }

    //kickstart ledger
    AddGenesisBlock();




    //init 100 online accounts
    //for(int i = 0; i < 100; i++)
    for(int i = 0; i < 100; i++)
    {
        Account a;
        a.Money = 100000;  //microalgos
        crypto_vrf_keypair(a.VRFKeys.VRFPubKey, a.VRFKeys.VRFPrivKey);
        OnlineAccounts.push_back(a);
    }

    InitBalanceTracker();
}


void ParticipationNode::AddGenesisBlock()
{
    LedgerEntry e;
    //e.SeedAndProof.Seed = "hgyurteydhsjaskeudiaoapdlfkruiu9";
    Ledger.Entries.push_back(e);
    round = 1;
}


void ParticipationNode::InitBalanceTracker()
{

}


#if SIMULATE_VRF

VRFOutput ParticipationNode::SimulateVRF()
{
    VRFOutput Out;
    uint256_t randomHash = generator();

    export_bits(randomHash, uchar_ptr(Out.VRFHash), 256);

    //get a proof that is just going to be either 0 or 1
    //we are abstracting away the VRF computation, to validate
    //other parts of the protocol
    Out.VRFProof = std::to_string(0).c_str();
    Out.VRFProof[79] = (unsigned char)(1);

    return Out;
}

#endif


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

#if SIMULATE_VRF
    cryptoDigest = SimulateVRF();
#else
    cryptoDigest = RunVRF(a, (unsigned char*)(m.c_str()), m.length());
#endif


    double binomialN = double(a.Money);
    double binomialP = expectedSize / double(totalMoney);

    BigInt t = byte_array_to_cpp_int(uchar_ptr(cryptoDigest.VRFHash), 64);

    EV << t << endl;

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


    AlgorandMessage* msg = AlgorandMessage::MakeMessage(ReusableMessages, round, 0, 0, SelectedBlock.ProposerCredentials, ChosenAccount->AccountAddress, SelectedBlock);
    Gossip(msg);
    AlgorandMessage::RecycleMessage(this, ReusableMessages, msg);


    //TODO: gossip full block


    return SelectedBlock;
}


void ParticipationNode::Gossip(AlgorandMessage* m)
{
    for (int i = 0; i < 1600; i++)  //1600
    {
        std::string nodePath = "AlgorandNetwork.PartNode[" + std::to_string(i) + "]";
        ParticipationNode* bro = (ParticipationNode*)(getModuleByPath(nodePath.c_str()));
        bro->P.push_back(m->Payload);
    }


//    int baseId = gateBaseId("gate$o"), size = gateSize("gate$o");
//    for (int i = 0; i < size; i++)
//    {
//        send(AlgorandMessage::DuplicateMessage(ReusableMessages, m), gate(baseId + i));
//    }
}


void ParticipationNode::ConfirmBlock(const LedgerEntry& hblock)
{
    //TODO: tipo de consenso. Por ahora, confirma el bloque que recibe
    //CountVotes(); //conteo de votos para ver

    Ledger.Entries.push_back(hblock);
}


void ParticipationNode::SoftVote()
{
    //podria ir comparando a medida que llegan e irme quedando con la mas chica para optimizar (seria compliant con los specs? revisar)

    for (auto& p : P)
    {
        if (p.cachedCredentialSVHash < pinnedPayload.e.cachedCredentialSVHash)
            pinnedPayload.e = p;
    }
}


void ParticipationNode::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) //timeout event
    {
        if (msg->getKind() == 0)
        {
            EV << "LLEGUE ACA" << endl;

            startTime = simTime();
            step = 0;

            //schedule a soft vote
            TimeoutEvent->setKind(1);
            scheduleAfter(FilterTimeout(period), TimeoutEvent);
            //schedule a fast resynchronization attempt
            scheduleAfter(LambdaF + 0, FastResyncEvent);




            auto LocalBlock = BlockAssembly();
            LocalBlock = BlockProposal(LocalBlock);

            //pin value? check on specs
            pinnedPayload.e = LocalBlock;




            //step = 1;
            lastConcludingStep = 0;
            return;  //aca ya propuse, y me quedo esperando el soft
        }
        else if (msg->getKind() == 1 || msg->getKind() == 2)  //2 no deberia ser nunca, pero lo incluyo por completitud
        {
            step = 1;

            TimeoutEvent->setKind(3);
            scheduleAt(startTime + fmax(4.f * Lambda, UppercaseLambda), TimeoutEvent);

            SoftVote();
            EV << "SOFT" << endl;

            step = 2;
            lastConcludingStep = 1;

            //certification vote happens from now on, but is driven by messages (not timeout)
            //node has to observe a cert bundle BEFORE the next timeout

            return;

        }
        else if (msg->getKind() < 253)  // >= 3 por definicion
        {
            lastConcludingStep = step;
            step = msg->getKind();

            TimeoutEvent->setKind(step + 1);
            scheduleAt(startTime + fmax(4.f * Lambda, UppercaseLambda) + pow(2.f, step) * Lambda + 0, TimeoutEvent);


            //nextVote()





            //por ahora (volver al loop):
            GarbageCollectState();
            TimeoutEvent->setKind(0);
            scheduleAfter(0, TimeoutEvent);




            return;
        }
        else //if (msg->getKind() >= 253)
        {
            lastConcludingStep = step;
            step = msg->getKind();

            scheduleAfter(LambdaF + 0, FastResyncEvent);

            //late, redo, down

            return;
        }
    }
//    else //vote or proposal message
//    {
//        AlgorandMessage* m = (AlgorandMessage*)(msg);
//
//        //TODO: validaciones varias (todas las causas para ignorar mensaje)
//        if (round > m->round)
//            AlgorandMessage::RecycleMessage(this, ReusableMessages, m);
//
//
//        //observar el mensaje
//        if (m->step == 0)
//        {
//            P.push_back(m->Payload);
//        }
//        else if (m->step == 1)
//        {
//            SoftVotes.push_back(m->vote);
//        }
//        else if (m->step == 2)
//        {
//            CertVotes.push_back(m->vote);
//        }
//
//        //observar el mensaje
//        //si se observa un nuevo round
//        //if (newRoundObs)
//        //{
//            //RescheduleAfter(Lambda, msg);
//        //}
//    }
}


void ParticipationNode::GarbageCollectState()
{
    P.clear();
    SoftVotes.clear();
    CertVotes.clear();
    RecoveryVotes.clear();

    cancelEvent(TimeoutEvent);
    cancelEvent(FastResyncEvent);

    period = 0;
}


void ParticipationNode::OnProposalReceived()
{

}


void ParticipationNode::OnVoteReceived()
{
    //validate  vote



}


void ParticipationNode::StartNewRound()
{
    lastConcludingStep = step;
    //pinnedValue = EMPTY;
    period = 0;
    step = 0;
    round++;
}


void ParticipationNode::StartNewPeriod()
{
    lastConcludingStep = step;
    step = 0;

    //PinnedValue = EMPTY;
    period++;
}
