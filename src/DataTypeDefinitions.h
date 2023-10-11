#ifndef DATATYPEDEFINITIONS_H_
#define DATATYPEDEFINITIONS_H_


/*
 * The following file contains definitions for most of the data structures and types
 * used across the whole simulation.
 * In some of them, a cached inner structure is used for simulation performance.
 * This behavior does not reflect the real node's behavior, who would usually
 * recompute these values every time they receive a new message. In a simulation
 * we will know which actors are honest and which are not, and thus we are allowed
 * to force recalculation only when relevant for the scenario being simulated,
 * making significant performance gains.
 */


#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <cstring>
#include <string>


typedef boost::multiprecision::uint256_t uint256_t;
typedef boost::multiprecision::cpp_int BigInt;
//typedef boost::multiprecision::number<boost::multiprecision::cpp_bin_float<156>> BigFloat; //2**(64*8) has 155 decimal digits
typedef boost::multiprecision::number<boost::multiprecision::cpp_bin_float<58*(256+1)>> BigFloat;

typedef uint64_t Address;


class BalanceRecord
{
public:
    uint64_t RawBalance;
    bool OnlineStatus;

    unsigned char VRFPubKey[32];

    BalanceRecord(){}
    BalanceRecord(uint64_t rb, bool status):RawBalance(rb), OnlineStatus(status)
    {
        old.OldBalance = rb;
        old.OldStatus = status;
    }


    //old cached stuff (for efficient lookup)
    struct stOld
    {
        uint64_t OldBalance;
        uint64_t OldStatus;

        //TODO: OLD VOTING
        unsigned char OldVRFPubKey[32];
    }old;
};


class Transaction
{
public:
    static uint64_t NEXT_ID;
    uint64_t txnID;


    Transaction(){txnID = NEXT_ID++;}

    enum TxnTypeEnum{PAY, KEYREG};
    TxnTypeEnum type;

    Address Sender;
    Address Receiver;

    uint64_t Amount;

    //VER TEMA FEES
    uint64_t Fee;


    uint64_t ValidFromRound;
    uint64_t ValidUntilRound;


    //keyreg stuff



    bool operator==(const Transaction& txn) const
    {
        return txnID == txn.txnID;
    }
};


class PayTransaction : public Transaction
{

};


class KeyregTransaction : public Transaction
{
    unsigned char VotePK[32];
    unsigned char SelectionPK[32];

    uint64_t voteFirst;
    uint64_t voteLast;

    //unused
    //stateProofPK
    //voteKeyDilution
    //nonparticipation
};


struct SignedTransaction
{
    Transaction* Txn;

    //signature
};




struct VRFKeyPair
{
    unsigned char VRFPubKey[32];
    unsigned char VRFPrivKey[64];
};


struct Account
{
    Account(Address addr, uint64_t m):I(addr), Money(m), BalanceMapPtr(nullptr){}
    Account(Address addr, uint64_t m, class BalanceRecord* br):I(addr), Money(m), BalanceMapPtr(br){}
    Address I;
    uint64_t Money;

    //TODO: voting
    //    struct stSecrets
    //    {
    //        //voting keys
    //        //TODO
    //
    //        unsigned char VRFPrivKey[64];
    //    }secrets;
    VRFKeyPair VRFKeys;


    //pointer to the balance map (useful for updating) all node managed accounts after finishing a round
    class BalanceRecord* BalanceMapPtr;


    //old stuff cached for performance reasons
    struct stOld
    {
        uint64_t OldBalance;
        bool OldStatus;

        //TODO voting
        VRFKeyPair OldVRFKeys;
    };
};


struct VRFOutput
{
    unsigned char VRFHash[64] = {0};
    unsigned char VRFProof[80] = {0};

    VRFOutput()
    {
    }


    bool operator==(const VRFOutput& a) const
    {
        return memcmp(VRFHash, a.VRFHash, 64) == 0 && memcmp(VRFProof, a.VRFProof, 80) == 0;
//        return (VRFHash == a.VRFHash && VRFProof == a.VRFProof);
    }

    bool operator!=(const VRFOutput& a) const
    {
        return memcmp(VRFHash, a.VRFHash, 64) != 0 || memcmp(VRFProof, a.VRFProof, 80) != 0;
//        return (VRFHash != a.VRFHash || VRFProof != a.VRFProof);
    }
};


struct stSeedAndProof
{
    std::string Seed;
    std::string Proof;

    stSeedAndProof()
    {
        Seed.resize(32, char(0));
        Proof.resize(32, char(0));
    }
};


struct LedgerEntry
{
    //used for speeding up comparison and indexing
    static uint64_t NEXT_ID;
    uint64_t LedgerEntryID;


    static LedgerEntry EMPTY_BLOCK;


    LedgerEntry(){LedgerEntryID = NEXT_ID++;}
    //header
    //TODO: ALL HEADER STUFF
//    stSeedAndProof SeedAndProof;
    unsigned char Seed[32];

    //body
    std::vector<Transaction*> Txns;

    VRFOutput ProposerCredentials;
    Address ProposerAddress;
    //unsigned char* Seed; //uint256_t Seed;  //Q


    uint256_t cachedCredentialSVHash;
    struct cached
    {
        unsigned char SeedProof[32];
    }cached;


    unsigned char* Encoding()
    {
        return nullptr;
    }

    unsigned char* Digest() //uint256_t Digest()
    {
        return nullptr;
    }
};


struct Ledger
{
    //Ledger params
//    uint64_t TMax = 1000; //length of the transaction tail
//    uint64_t BMax = 5242880; //maximum number of txn bytes per block
//    uint64_t bMin = 100000; //minimum balance for an address


    bool ValidEntry(LedgerEntry e){return true; }
    unsigned char* SeedLookup(uint64_t round){return Entries[round>0? round : 0].Seed;}
    //TODO RecordLookup
    unsigned char* DigestLookup(uint64_t round){return Entries[round>0? round : 0].Digest();} //uint256_t DigestLookup(uint64_t round){return Entries[round].Digest();}
    //TODO TotalStakeLookup


    std::vector<LedgerEntry> Entries;
};


struct ProposalPayload
{
    //used for speeding up comparison and indexing
    static uint64_t NEXT_ID;
    uint64_t ProposalPayloadID;
    uint64_t RoundOfAssembly;


    LedgerEntry e;
    uint256_t y;   //signature

    //de acuerdo con messageDecodeFilter_test.go esto va acá también
    Address I_orig;
    uint64_t p_orig;


    ProposalPayload(uint64_t r):RoundOfAssembly(r){ProposalPayloadID=NEXT_ID++;}
    ProposalPayload(uint64_t r, LedgerEntry& entry):RoundOfAssembly(r),e(entry){ProposalPayloadID=NEXT_ID++;}


    struct performanceCachedStuff
    {
        //uint256_t d; //digest(e), useful for looking up all votes associated with this proposal
        uint64_t d;
    }Cached;
};


struct ProposalValue
{
    ProposalValue():I_orig(0), p_orig(0), d(0) {Cached.weight = 0; Cached.lowestComputedHash = 0;}
    ProposalValue(Address I, uint64_t p, uint64_t dig):I_orig(I), p_orig(p), d(dig){}

    Address I_orig;
    uint64_t p_orig;

//    uint256_t d;
    uint64_t d;

//    struct unused
//    {
//        uint256_t h;  //Hash(encoding(e))
//    };


    struct performanceCachedStuff
    {
        VRFOutput Credentials;
        uint64_t weight;

        uint256_t lowestComputedHash;
    }Cached;


    bool operator==(const ProposalValue& a) const
    {
        return (I_orig == a.I_orig && p_orig == a.p_orig && d == a.d); //&& Cached.Credentials == a.Cached.Credentials && Cached.lowestComputedHash == a.Cached.lowestComputedHash);
    }

    bool operator!=(const ProposalValue& a) const
    {
        return (I_orig != a.I_orig || p_orig != a.p_orig || d != a.d); //|| Cached.Credentials != a.Cached.Credentials || Cached.lowestComputedHash != a.Cached.lowestComputedHash);
    }
};


struct Vote
{
    //used for speeding up comparison
    static uint64_t NEXT_ID;
    uint64_t voteID;


    //raw vote
    Address I;
    uint64_t r;
    uint64_t p;
    uint8_t s;
    ProposalValue v;

    //Verified credentials:
    VRFOutput VRFOut;
    uint64_t weight;

    unsigned char signature[64];

    bool operator == (const Vote& rhs) const
    {
        return voteID == rhs.voteID;
//        return I == rhs.I && r == rhs.r && p == rhs.p && s == rhs.s && v == rhs.v && weight == rhs.weight;
    }

    Vote():I(0), r(0), p(0), s(0), weight(0){voteID = NEXT_ID++;}
    Vote(Address addr, uint64_t round, uint64_t period, uint8_t step, ProposalValue propVal, VRFOutput cred, uint64_t j):
        I(addr), r(round), p(period), s(step), v(propVal), VRFOut(cred), weight(j){voteID = NEXT_ID++;}
};


struct EquivocationVote
{
    Vote* voteA;
    Vote* voteB;

//    static bool IsEquivocation()
};


struct Bundle
{
    //used for speeding up comparison
    static uint64_t NEXT_ID;
    uint64_t bundleID;


    uint64_t weight;
    std::vector<Vote*> votes;

    Bundle():weight(0){bundleID = NEXT_ID++; }
};


enum MsgType{TXN, PROPOSAL, VOTE, BUNDLE};


inline unsigned int CommitteeSize(uint8_t Step)
{
    switch(Step)
    {
        case 0:   return 20;
        case 1:   return 2990;
        case 2:   return 1500;
        case 253: return 500;
        case 254: return 2400;
        case 255: return 6000;
        default:  return 5000; //2 < Step < 253
    }
}


inline unsigned int CommitteeThreshold(uint8_t Step)
{
    switch(Step)
    {
        case 0:   return 0;
        case 1:   return 2267;
        case 2:   return 1112;
        case 253: return 320;
        case 254: return 1768;
        case 255: return 4560;
        default:  return 3838; //2 < Step < 253
    }
}




//-------------------- Conversion helper functions --------------------//
static BigInt byte_array_to_cpp_int(unsigned char* n, uint64_t len)
{
    BigInt i;
    import_bits(i, n, n+len);
    return i;
}


static inline unsigned char* uchar_ptr(std::string& str){return (unsigned char*)(str.data());}


#endif
