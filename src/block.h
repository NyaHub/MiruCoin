#include "./json.hpp"
#include "./utils.h"
#include "merkletree.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <openssl/sha.h>
#include <string>

namespace Mirucoin {
struct BlockEntity {
  int id;
  std::string merkleRoot;
  std::string hash;
  std::string prevHash;
  uint64_t nonce;
  uint64_t index;
  time_t timestamp;
  std::string data;
};

class Block {
private:
  merkle::Tree tree;
  std::string hash;
  std::string prevHash;
  uint64_t nonce;
  uint64_t index;
  time_t timestamp;
  std::string data;
  nlohmann::json header;

public:
  Block(std::string prevHash, uint64_t index, std::string data) {
    this->prevHash = prevHash;
    this->data = data;
    this->index = index;
    this->timestamp = std::time(nullptr);

    std::srand(this->timestamp);

    this->nonce = std::rand();

    std::string hashedData = sha256String(this->data);

    this->header = {
        {"prevHash", this->prevHash},
        {"index", this->index},
        {"timestamp", this->timestamp},
        {"nonce", this->nonce},
        {"merkleRoot", hashedData}
        // {"merkleRoot", this->tree.root().to_string()}
    };

    this->hash = sha256String(this->header.dump());
  }
  Block(std::string prevHash, uint64_t index, std::string data,
        time_t timestamp, uint64_t nonce) {
    this->prevHash = prevHash;
    this->nonce = nonce;
    this->index = index;
    this->timestamp = timestamp;
    this->data = data;

    std::string hashedData = sha256String(this->data);

    this->header = {
        {"prevHash", this->prevHash},
        {"index", this->index},
        {"timestamp", this->timestamp},
        {"nonce", this->nonce},
        {"merkleRoot", hashedData}
        // {"merkleRoot", this->tree.root().to_string()}
    };

    this->hash = sha256String(this->header.dump());
  }

  void updateHeader(uint64_t nonce) {
    header["nonce"] = nonce;
    this->nonce = nonce;
  }

  const std::string &getHash() { return hash; }
  const std::string &getPrevHash() { return prevHash; }
  const std::string &getData() { return data; }
  std::string getMerkleRoot() { return sha256String(this->data); }
  //   std::string getMerkleRoot() { return std::move(tree.root().to_string());}
  std::string getHeader() { return std::move(header.dump()); }
  uint64_t getNonce() { return nonce; }
  uint64_t getIndex() { return index; }
  time_t getTimestamp() { return timestamp; }

  std::string toString() {
    nlohmann::json blk = {{"header", header}, {"data", data}};
    return blk.dump();
  }

  static std::unique_ptr<Block> createGenesis() {
    return std::make_unique<Block>("", 0, "this is genesis block");
  }

  static std::unique_ptr<Block> createBlock(std::string prevHash,
                                            uint64_t index, std::string data) {
    return std::make_unique<Block>(prevHash, index, data);
  }
  static std::unique_ptr<Block> createBlock(BlockEntity *blk) {
    return std::make_unique<Block>(blk->prevHash, blk->index, blk->data,
                                   blk->timestamp, blk->nonce);
  }

  static int verify(Block *blk, Block *lastblk) {
    if (blk->getPrevHash() == lastblk->getHash()) {
      return 1;
    } else {
      return 0;
    }
  }

  void print() {
    printf("Block %lu\n\thash: %s\n\tprevious hash: %s\n\tnonce: "
           "%lu\n\ttimestamp: %lu\n\tdata: %s\n",
           index, hash.c_str(), prevHash.c_str(), nonce, timestamp,
           data.c_str());
  }
};
} // namespace Mirucoin
