#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

typedef boost::multiprecision::uint256_t uint256_t;


typedef unsigned int simpleBlock;


struct VRFKeyPair
{
    unsigned char VRFPubKey[32];
    unsigned char VRFPrivKey[64];
};


struct Account
{
    uint64_t Address;
    uint64_t Money;

    VRFKeyPair VRFKeys;
};


struct VRFOutput
{
    unsigned char VRFProof[80];
    unsigned char VRFHash[64];
};


struct Ledger
{
    std::vector<unsigned int> blocks;
};






struct LedgerEntry
{
    //o is some opaque object O
    unsigned char* Obj;
    uint256_t Seed;  //Q

    unsigned char* Encoding()
    {
        return nullptr;
    }

    uint256_t Digest()
    {
        return 0;
    }
};


struct NewLedger
{
    bool ValidEntry(LedgerEntry e){return true; }
    uint256_t SeedLookup(uint64_t round){return Entries[round].Seed;}
    //TODO RecordLookup
    uint256_t DigestLookup(uint64_t round){return Entries[round].Digest();}
    //TODO TotalStakeLookup


    std::vector<LedgerEntry> Entries;
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

