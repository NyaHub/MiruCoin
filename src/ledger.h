#include <crypto++/config_int.h>
#include <crypto++/hex.h>
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
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey pk;
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey pubk;

public:
  Ledger() {
    pk.Initialize(prng, CryptoPP::ASN1::secp256k1());
    pk.MakePublicKey(pubk);
  }

  std::string sign(std::string message) {
    CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Signer signer(pk);

    std::string signature;

    CryptoPP::StringSource s(
        message, true,
        new CryptoPP::SignerFilter(
            prng, signer,
            new CryptoPP::HexEncoder(new CryptoPP::StringSink(signature))));

    return signature;
  }

  std::string getDerPublic() {
    std::string epubk;
    CryptoPP::HexEncoder enc;
    enc.Attach(new CryptoPP::StringSink(epubk));
    pubk.Save(enc);
    return epubk;
  }
  std::string getDerPrivate() {
    std::string epk;
    CryptoPP::HexEncoder enc;
    enc.Attach(new CryptoPP::StringSink(epk));
    pk.Save(enc);
    return epk;
  }

  auto verify(std::string message, std::string hexsignature,
              std::string derpublickey) {
    CryptoPP::HexDecoder dec;
    dec.Put((CryptoPP::byte *)derpublickey.c_str(), derpublickey.size());
    dec.MessageEnd();

    CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::Verifier verifier;
    verifier.AccessKey().Load(dec);
    bool result = false;

    std::string signature;
    CryptoPP::StringSource signss(
        hexsignature, true,
        new CryptoPP::HexDecoder(new CryptoPP::StringSink(signature)));

    CryptoPP::StringSource outss(
        signature + message, true,
        new CryptoPP::SignatureVerificationFilter(
            verifier, new CryptoPP::ArraySink((CryptoPP::byte *)&result,
                                              sizeof(result))));
    return result;
  }
};

} // namespace Mirucoin