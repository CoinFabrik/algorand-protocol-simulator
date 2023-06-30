#ifndef DATATYPEDEFINITIONS_H_
#define DATATYPEDEFINITIONS_H_

#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <cstring>
#include <string>


typedef boost::multiprecision::uint256_t uint256_t;
typedef boost::multiprecision::cpp_int BigInt;
typedef boost::multiprecision::number<boost::multiprecision::cpp_bin_float<156>> BigFloat; //2**(64*8) has 155 decimal digits


typedef uint256_t Address;


class BalanceRecord
{
public:
    uint64_t RawBalance;
    bool OnlineStatus;

    //TODO: keys

    BalanceRecord(){}
    BalanceRecord(uint64_t rb, bool status):RawBalance(rb), OnlineStatus(status), OldBalance(0), OldStatus(true){}

    //cached for lookup
    uint64_t OldBalance;
    uint64_t OldStatus;

    //TODO: old keys
};


//transaction stuff
//work in progress
//enum txnType {};
class Transaction
{
public:
    Transaction(){}

    uint256_t txnID;
    //txnType type;  //por ahora, solo pay

    Address Sender;
    Address Receiver;

    uint64_t Amount;

    //VER TEMA FEES
    uint64_t Fee;
};


class SignedTransaction
{
    Transaction Txn;

    //signature
};




struct VRFKeyPair
{
    unsigned char VRFPubKey[32];
    unsigned char VRFPrivKey[64];
};


struct Account
{
    Account(Address addr, uint64_t m):I(addr), Money(m){}
    Address I;
    uint64_t Money;

    VRFKeyPair VRFKeys;
};


struct VRFOutput
{
    unsigned char VRFHash[64] = {0};
    unsigned char VRFProof[80] = {0};
    //std::string VRFHash;
    //std::string VRFProof;

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
    LedgerEntry()
    {

    }
    LedgerEntry(int v):PlaceholderID(v){}


    //o is some opaque object O
    //unsigned char* Obj;
    int PlaceholderID;

    //header
    //TODO: ALL HEADER STUFF
    stSeedAndProof SeedAndProof;

    //body
    std::vector<Transaction> Txns;

    VRFOutput ProposerCredentials;
    Address ProposerAddress;
    //unsigned char* Seed; //uint256_t Seed;  //Q


    uint256_t cachedCredentialSVHash;


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
    uint64_t TMax = 1000; //length of the transaction tail
    uint64_t BMax = 5242880; //maximum number of txn bytes per block
    uint64_t bMin = 100000; //minimum balance for an address


    bool ValidEntry(LedgerEntry e){return true; }
    std::string SeedLookup(uint64_t round){return Entries[round>0? round : 0].SeedAndProof.Seed;}
    //TODO RecordLookup
    unsigned char* DigestLookup(uint64_t round){return Entries[round>0? round : 0].Digest();} //uint256_t DigestLookup(uint64_t round){return Entries[round].Digest();}
    //TODO TotalStakeLookup


    std::vector<LedgerEntry> Entries;

    //chanteada
    std::vector<uint256_t> SimEntries;
};


struct ProposalPayload
{
    LedgerEntry e;
    uint256_t y;   //signature

    //de acuerdo con messageDecodeFilter_test.go esto va acá también
    Address I_orig;
    uint64_t p_orig;


    ProposalPayload(){}
    ProposalPayload(LedgerEntry& entry):e(entry){}



    struct performanceCachedStuff
    {
        uint256_t d; //digest(e), useful for looking up all votes associated with this proposal
    }Cached;
};


struct ProposalValue
{
    ProposalValue()
    {
        I_orig = 0; p_orig = 0; d = 0; Cached.weight = 0; Cached.lowestComputedHash = 0;

    }


    Address I_orig;
    uint64_t p_orig;

    //uint64_t d;   //digest(e), o sea Hash(e)
    uint256_t d;

    struct unused
    {
        uint256_t h;  //Hash(encoding(e))
    };

    
    struct performanceCachedStuff
    {
        VRFOutput Credentials;
        uint64_t weight;

        uint256_t lowestComputedHash;
    }Cached;


    bool operator==(const ProposalValue& a) const
    {
        return (I_orig == a.I_orig && p_orig == a.p_orig && d == a.d && Cached.Credentials == a.Cached.Credentials && Cached.lowestComputedHash == a.Cached.lowestComputedHash);
    }

    bool operator!=(const ProposalValue& a) const
    {
        return (I_orig != a.I_orig || p_orig != a.p_orig || d != a.d || Cached.Credentials != a.Cached.Credentials || Cached.lowestComputedHash != a.Cached.lowestComputedHash);
    }
};


struct Vote
{
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
        return I == rhs.I && r == rhs.r && p == rhs.p && s == rhs.s && v == rhs.v && weight == rhs.weight;
    }

    Vote():I(0), r(0), p(0), s(0), weight(0){}
    Vote(Address addr, uint64_t round, uint64_t period, uint8_t step, ProposalValue propVal, VRFOutput cred, uint64_t j):
        I(addr), r(round), p(period), s(step), v(propVal), VRFOut(cred), weight(j){}
};


struct EquivocationVote
{
    Vote* voteA;
    Vote* voteB;

//    static bool IsEquivocation()
};


struct Bundle
{
    uint64_t weight;
    std::vector<Vote> votes;

    Bundle():weight(0){}
};


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







//some network level stuff
struct Relay
{
    //in this model, relay nodes act as connections in between participation nodes
    //if there's no route between two nodes, they can't send messages to each other

    //acts as an address
    uint64_t RelayID;

    //list of part nodes to send messages to / get messages from
    //TODO

    std::vector<int> RelayConnections;          //Relay node IDs in network list
    std::vector<int> ParticipationConnections;  //Participation node IDs in network list

    //TODO: some stats (reception delay time, output delay time, etc.)
};


enum MsgType{TXN, PROPOSAL, VOTE, BUNDLE};


#endif
