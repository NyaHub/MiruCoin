#include "./json.hpp"
#include "merkletree.hpp"
#include <cstdint>
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

    this->header = {
        {"prevHash", this->prevHash},
        {"index", this->index},
        {"timestamp", this->timestamp},
        {"nonce", this->nonce},
        // {"merkleRoot", this->tree.root().to_string()}
    };
  }
  void updateHeader(uint64_t nonce) { header["nonce"] = nonce; }
  const std::string &getHash() { return hash; }
  const std::string &getPrevHash() { return prevHash; }
  const std::string &getData() { return data; }
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
};
} // namespace Mirucoin
