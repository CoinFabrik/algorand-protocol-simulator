#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>

typedef boost::multiprecision::uint256_t uint256_t;
typedef boost::multiprecision::cpp_int BigInt;
typedef boost::multiprecision::number<boost::multiprecision::cpp_bin_float<156>> BigFloat; //2**(64*8) has 155 decimal digits


typedef uint256_t Address;


struct VRFKeyPair
{
    unsigned char VRFPubKey[32];
    unsigned char VRFPrivKey[64];
};


struct Account
{
    Address AccountAddress;
    uint64_t Money;

    VRFKeyPair VRFKeys;
};


struct VRFOutput
{
    std::string VRFHash;
    std::string VRFProof;

    VRFOutput()
    {
        VRFHash.resize(64, char(0));
        VRFProof.resize(80, char(0));
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
    //o is some opaque object O
    //unsigned char* Obj;
    int PlaceholderID;

    stSeedAndProof SeedAndProof;

    VRFOutput ProposerCredentials;
    Address ProposerAddress;
    //unsigned char* Seed; //uint256_t Seed;  //Q


    LedgerEntry()
    {

    }
    LedgerEntry(int v):PlaceholderID(v){}

    ~LedgerEntry()
    {

    }

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
    bool ValidEntry(LedgerEntry e){return true; }
    std::string SeedLookup(uint64_t round){return Entries[round>0? round : 0].SeedAndProof.Seed;}
    //TODO RecordLookup
    unsigned char* DigestLookup(uint64_t round){return Entries[round>0? round : 0].Digest();} //uint256_t DigestLookup(uint64_t round){return Entries[round].Digest();}
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




//-------------------- Conversion helper functions --------------------//
static BigInt byte_array_to_cpp_int(unsigned char* n, uint64_t len)
{
    BigInt i;
    import_bits(i, n, n+len);
    return i;
}


static inline unsigned char* uchar_ptr(std::string& str){return (unsigned char*)(str.data());}
