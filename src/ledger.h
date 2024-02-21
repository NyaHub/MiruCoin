#ifndef LEDGER_LIB
#define LEDGER_LIB
#include "tx.h"
#include "utils.h"
#include <crypto++/config_int.h>
#include <crypto++/hex.h>
#include <crypto++/oids.h>
#include <crypto++/queue.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/xed25519.h>
#include <optional>
#include <string>

namespace Mirucoin {

class LedgerEntity {
public:
  std::string address;
  float ballance;
  float lockedOut;
  float lockedIn;
  LedgerEntity() {
    this->address = "";
    this->ballance = 0;
    this->lockedOut = 0;
    this->lockedIn = 0;
  }
  LedgerEntity(std::string addr, float ballance = 0, float lockedOut = 0,
               float lockedIn = 0) {
    this->address = addr;
    this->ballance = ballance;
    this->lockedOut = lockedOut;
    this->lockedIn = lockedIn;
  }
  LedgerEntity(const LedgerEntity &a) {
    this->address = a.address;
    this->ballance = a.ballance;
    this->lockedOut = a.lockedOut;
    this->lockedIn = a.lockedIn;
  }
  void print() {
    printf("Wallet: %s\n\tBallance: %f\n", address.c_str(), ballance);
  }
};

class Ledger {
private:
  CryptoPP::AutoSeededRandomPool prng;
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey pk;
  CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey pubk;
  LedgerEntity params;

public:
  Ledger() {
    pk.Initialize(prng, CryptoPP::ASN1::secp256k1());
    pk.MakePublicKey(pubk);
    params.address = "0x" + keccak256String(getDerPublic()).substr(0, 40);
  }

  void update(LedgerEntity params) {
    if (params.address != this->params.address)
      return;
    this->params.ballance = params.ballance;
    this->params.lockedOut = params.lockedOut;
    this->params.lockedIn = params.lockedIn;
  }

  const std::string &getAddress() { return params.address; }
  const float getBallance() { return params.ballance; }
  const float getLockedOut() { return params.lockedOut; }
  const float getLockedIn() { return params.lockedIn; }

  std::optional<Tx> send(std::string addr, float amount, float fee,
                         std::string data = "") {
    if (amount + fee > params.ballance) {
      return {};
    }

    Tx tx{params.address, addr, amount, fee, data};
    tx.setSign(sign(tx.getHash()));
    return tx;
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

  static auto verify(std::string message, std::string hexsignature,
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

  void print() {
    printf("Wallet: %s\n\tBallance: %f\n", params.address.c_str(),
           params.ballance);
  }
};

} // namespace Mirucoin
#endif