#ifndef BLOCK_LIB
#define BLOCK_LIB
#include "./json.hpp"
#include "./tx.h"
#include "./utils.h"
#include "merkletree.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <openssl/sha.h>
#include <string>
#include <vector>

namespace Mirucoin {
struct BlockEntity {
  int id;
  std::string merkleRoot;
  std::string hash;
  std::string prevHash;
  uint64_t nonce;
  uint64_t index;
  uint diff;
  time_t timestamp;
};

class Block {
private:
  merkle::Tree tree;
  std::string hash;
  std::string prevHash;
  uint64_t nonce;
  uint64_t index;
  time_t timestamp;
  std::vector<Tx> data;
  nlohmann::json header;
  uint diff;

public:
  Block(Block const &a) {
    this->prevHash = a.prevHash;
    this->nonce = a.nonce;
    this->index = a.index;
    this->timestamp = a.timestamp;
    this->data = a.data;
    this->diff = a.diff;

    for (int i = 0; i < this->data.size(); i++) {
      tree.insert(merkle::Tree::Hash(data[i].getHash()));
    }

    this->header = {{"prevHash", this->prevHash},
                    {"index", this->index},
                    {"timestamp", this->timestamp},
                    {"nonce", this->nonce},
                    {"merkleRoot", this->tree.root().to_string()},
                    {"difficulty", this->diff}};

    this->hash = sha512String(this->header.dump());

    // if (a.hash != this->hash) throw std::exception()
  };
  Block(std::string prevHash, uint64_t index, std::vector<Tx> data, uint diff) {
    this->prevHash = prevHash;
    this->data = data;
    this->index = index;
    this->timestamp = std::time(nullptr);
    this->diff = diff;

    this->nonce = std::rand();

    for (int i = 0; i < this->data.size(); i++) {
      tree.insert(merkle::Tree::Hash(this->data[i].getHash()));
    }

    this->header = {{"prevHash", this->prevHash},
                    {"index", this->index},
                    {"timestamp", this->timestamp},
                    {"nonce", this->nonce},
                    {"merkleRoot", this->tree.root().to_string()},
                    {"difficulty", this->diff}};

    this->hash = sha512String(this->header.dump());
  }
  Block(std::string prevHash, uint64_t index, std::vector<Tx> data, uint diff,
        time_t timestamp, uint64_t nonce) {
    this->prevHash = prevHash;
    this->nonce = nonce;
    this->index = index;
    this->timestamp = timestamp;
    this->data = data;
    this->diff = diff;

    for (int i = 0; i < this->data.size(); i++) {
      tree.insert(merkle::Tree::Hash(data[i].getHash()));
    }

    this->header = {{"prevHash", this->prevHash},
                    {"index", this->index},
                    {"timestamp", this->timestamp},
                    {"nonce", this->nonce},
                    {"merkleRoot", this->tree.root().to_string()},
                    {"difficulty", this->diff}};

    this->hash = sha512String(this->header.dump());
  }

  void updateHeader(uint64_t nonce) {
    header["nonce"] = nonce;
    this->nonce = nonce;
    this->hash = sha512String(this->header.dump());
  }

  const std::string &getHash() { return hash; }
  const std::string &getPrevHash() { return prevHash; }
  const std::vector<Tx> &getData() { return data; }
  const std::string getMerkleRoot() { return tree.root().to_string(); }
  const std::string getHeader() { return header.dump(); }
  const uint64_t getNonce() { return nonce; }
  const uint64_t getIndex() { return index; }
  const time_t getTimestamp() { return timestamp; }
  const uint getDifficulty() { return diff; }

  nlohmann::json toJson() {
    std::vector<nlohmann::json> jdata;
    for (int i = 0; i < data.size(); i++) {
      jdata.push_back(data[i].toJson());
    }
    nlohmann::json blk = {{"header", header}, {"data", jdata}};
    return blk;
  }

  std::string toString() {
    std::vector<nlohmann::json> jdata;
    for (int i = 0; i < data.size(); i++) {
      jdata.push_back(data[i].toJson());
    }
    nlohmann::json blk = {{"header", header}, {"data", jdata}};
    return blk.dump();
  }

  static Block createGenesis(uint diff) {
    std::vector<Tx> txs;
    txs.push_back(Tx("me", "you", 10, 10, "GENESIS"));
    return Block("", 0, std::move(txs), diff);
  }

  static Block createBlock(std::string prevHash, uint64_t index,
                           std::vector<Tx> data, uint diff) {
    return Block(prevHash, index, std::move(data), diff);
  }
  static Block createBlock(BlockEntity *blk, std::vector<Tx> data) {
    return Block(blk->prevHash, blk->index, std::move(data), blk->diff,
                 blk->timestamp, blk->nonce);
  }

  static int verify(Block &blk, Block &lastblk) {
    if (blk.getPrevHash() == lastblk.getHash()) {
      return 1;
    } else {
      return 0;
    }
  }

  static bool chekHash(Block &blk) {
    for (int i = 0; i < blk.getDifficulty(); i++) {
      if (blk.getHash()[i] != '0') {
        return false;
      }
    }
    return true;
  }

  void print() {
    printf("Block %lu\n\thash: %s\n\tprevious hash: %s\n\tnonce: "
           "%lu\n\ttimestamp: %lu\n\tmerkleRoot: %s\n\tdiff: %i\n",
           index, hash.c_str(), prevHash.c_str(), nonce, timestamp,
           tree.root().to_string().c_str(), diff);
  }
};
} // namespace Mirucoin
#endif