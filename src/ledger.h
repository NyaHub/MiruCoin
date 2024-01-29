#include <crypto++/oids.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/xed25519.h>
#include <string>

namespace Mirucoin {

class Ledger {
private:
  CryptoPP::AutoSeededRandomPool prng;
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Signer signer;

public:
  Ledger() { signer.AccessKey().Initialize(prng, CryptoPP::ASN1::secp256k1()); }

  std::string sign(std::string message) {
    std::string signature;

    CryptoPP::StringSource s(
        message, true,
        new CryptoPP::SignerFilter(prng, signer,
                                   new CryptoPP::StringSink(signature)));

    return signature;
  }

  auto verify(std::string message, std::string signature,
              std::string derpublickey) {
    CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey publicKey;

    CryptoPP::StringSink sspublic(derpublickey);

    CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Verifier verifier;
    verifier.AccessKey().DEREncode(sspublic);
    bool result = false;

    CryptoPP::StringSource ss(
        signature + message, true,
        new CryptoPP::SignatureVerificationFilter(
            verifier, new CryptoPP::ArraySink((CryptoPP::byte *)&result,
                                              sizeof(result))));
    return result;
  }
};

} // namespace Mirucoin