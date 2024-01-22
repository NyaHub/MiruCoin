#include "./utils.h"
#include "cryptopp/sha.h"
#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <string>

std::string sha256(std::string message) {

  std::string digest;

  CryptoPP::HexEncoder encoder(new CryptoPP::FileSink(std::cout));

  CryptoPP::SHA256 hash;

  hash.Update((const CryptoPP::byte *)message.data(), message.size());
  digest.resize(hash.DigestSize());
  hash.Final((CryptoPP::byte *)&digest[0]);

  CryptoPP::StringSource ss(digest, true, new CryptoPP::Redirector(encoder));

  return digest;
}

std::string sha256String(std::string message) {

  std::string digest;

  CryptoPP::HexEncoder encoder;

  CryptoPP::SHA256 hash;

  hash.Update((const CryptoPP::byte *)message.data(), message.size());
  digest.resize(hash.DigestSize());
  hash.Final((CryptoPP::byte *)&digest[0]);

  std::string encoded;

  CryptoPP::StringSource ss(
      digest, true,
      new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)));

  return encoded;
}

std::string sha512(std::string message) {

  std::string digest;

  CryptoPP::HexEncoder encoder(new CryptoPP::FileSink(std::cout));

  CryptoPP::SHA512 hash;

  hash.Update((const CryptoPP::byte *)message.data(), message.size());
  digest.resize(hash.DigestSize());
  hash.Final((CryptoPP::byte *)&digest[0]);

  CryptoPP::StringSource ss(digest, true, new CryptoPP::Redirector(encoder));

  return digest;
}

std::string sha512String(std::string message) {

  std::string digest;

  CryptoPP::HexEncoder encoder;

  CryptoPP::SHA512 hash;

  hash.Update((const CryptoPP::byte *)message.data(), message.size());
  digest.resize(hash.DigestSize());
  hash.Final((CryptoPP::byte *)&digest[0]);

  std::string encoded;

  CryptoPP::StringSource ss(
      digest, true,
      new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)));

  return encoded;
}

std::string sha2toString(std::string digest) {
  std::string encoded;

  CryptoPP::StringSource ss(
      digest, true,
      new CryptoPP::HexEncoder(new CryptoPP::StringSink(encoded)));

  return encoded;
}