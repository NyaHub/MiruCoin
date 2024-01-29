#include "./block.h"
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sqlite_orm/sqlite_orm.h>
#include <string>
#include <utility>
#include <vector>

namespace Mirucoin {
enum ChainStatus {
  Ok,
  DB_Error, // can't open database
  VerificationError,
  InsertError,
};

auto createDB(std::string db_name) {
  return sqlite_orm::make_storage(
      db_name,
      sqlite_orm::make_table(
          "Blocks",
          sqlite_orm::make_column("id", &BlockEntity::id,
                                  sqlite_orm::primary_key().autoincrement()),
          sqlite_orm::make_column("merkleRoot", &BlockEntity::merkleRoot),
          sqlite_orm::make_column("hash", &BlockEntity::hash),
          sqlite_orm::make_column("prevHash", &BlockEntity::prevHash),
          sqlite_orm::make_column("nonce", &BlockEntity::nonce),
          sqlite_orm::make_column("index", &BlockEntity::index),
          sqlite_orm::make_column("timestamp", &BlockEntity::timestamp),
          sqlite_orm::make_column("data", &BlockEntity::data)));
}

class Blockchain {
private:
  std::unique_ptr<Block> lastBlock;
  int length = 0;
  uint diff;
  ChainStatus status = Ok;
  decltype(createDB(std::declval<std::string>())) db;
  int addToDB(Block *blk) {
    BlockEntity block{-1,
                      blk->getMerkleRoot(),
                      blk->getHash(),
                      blk->getPrevHash(),
                      blk->getNonce(),
                      blk->getIndex(),
                      blk->getTimestamp(),
                      blk->getData()};
    return db.insert(block);
  }
  bool scanDB() {
    std::vector<BlockEntity> blks = db.get_all<BlockEntity>();

    bool flag = false;

    for (int i = 0; i < blks.size(); i++) {
      std::unique_ptr<Block> blk = Block::createBlock(&blks[i]);
      if (blk->getPrevHash() == "" && !flag) {
        this->lastBlock = std::move(blk);
        flag = true;
        length++;
      } else if (Block::verify(blk.get(), lastBlock.get())) {
        this->lastBlock = std::move(blk);
        length++;
      } else {
        db.remove<BlockEntity>(blks[i].id);
      }
    }
    return flag;
  }

public:
  Blockchain(std::string db_name, uint diff = 1) : db{createDB(db_name)} {
    db.sync_schema();

    if (!scanDB()) {
      auto blk = Block::createGenesis();
      addBlock(blk.get());
    }

    this->diff = diff;
  };

  std::string getlastHash() { return lastBlock->getHash(); }
  uint64_t getLastIndex() { return lastBlock->getIndex() + 1; }

  ChainStatus addBlock(Block *blk) {
    bool flag = length == 0 ? true : Block::verify(blk, lastBlock.get());

    if (flag) {
      if (this->addToDB(blk)) {
        this->lastBlock = (std::unique_ptr<Block>)blk;
        return Ok;
        length++;
      } else {
        return InsertError;
      }
    } else {
      return VerificationError;
    }
  }
  int getStatus() { return status; }
  void print() {
    std::vector<BlockEntity> blks = db.get_all<BlockEntity>();

    printf("Chain length: %lu\n", blks.size());

    for (int i = 0; i < blks.size(); i++) {
      std::unique_ptr<Block> blk = Block::createBlock(&blks[i]);
      blk->print();
    }
  }
};
} // namespace Mirucoin
