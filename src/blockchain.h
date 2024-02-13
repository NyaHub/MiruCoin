#ifndef BLOCKCHAIN_LIB
#define BLOCKCHAIN_LIB

#include "./block.h"
#include "./tx.h"
#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
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

inline auto createDB(std::string db_name) {
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
          sqlite_orm::make_column("timestamp", &BlockEntity::timestamp)),
      sqlite_orm::make_table(
          "Txs",
          sqlite_orm::make_column("id", &TxEntity::id,
                                  sqlite_orm::primary_key().autoincrement()),
          sqlite_orm::make_column("payer", &TxEntity::payer),
          sqlite_orm::make_column("payee", &TxEntity::payee),
          sqlite_orm::make_column("amount", &TxEntity::amount),
          sqlite_orm::make_column("fee", &TxEntity::fee),
          sqlite_orm::make_column("data", &TxEntity::data),
          sqlite_orm::make_column("timestamap", &TxEntity::timestamp),
          sqlite_orm::make_column("blockId", &TxEntity::blockId),
          sqlite_orm::foreign_key(&TxEntity::blockId)
              .references(&BlockEntity::id)));
}

class Blockchain {
private:
  std::optional<Block> lastBlock;
  int length = 0;
  uint diff;
  ChainStatus status = Ok;
  decltype(createDB(std::declval<std::string>())) db;
  ChainStatus addToDB(Block *blk) {
    try {
      db.begin_transaction();
      BlockEntity block{-1,
                        blk->getMerkleRoot(),
                        blk->getHash(),
                        blk->getPrevHash(),
                        blk->getNonce(),
                        blk->getIndex(),
                        blk->getTimestamp()};
      block.id = db.insert(block);

      auto &txs = blk->getData();

      std::cout << block.id << std::endl;

      for (int i = 0; i < txs.size(); i++) {
        db.insert(TxEntity{-1, txs[i].getPayer(), txs[i].getPayee(),
                           txs[i].getAmount(), txs[i].getFee(),
                           txs[i].getData(), txs[i].getTimestamp(),
                           std::make_unique<int>(block.id)});
      }
      db.commit();
    } catch (...) {
      db.rollback();
      std::cout << "DB Error" << std::endl;
      return DB_Error;
    }
    return Ok;
  }

  bool scanDB() {
    std::vector<BlockEntity> blks = db.get_all<BlockEntity>();

    bool flag = false;

    for (int i = 0; i < blks.size(); i++) {
      std::vector<Tx> txs = getTxsByBlkId(blks[i].id);
      Block blk = Block::createBlock(&blks[i], std::move(txs));
      if (blk.getPrevHash() == "" && !flag) {
        this->lastBlock = blk;
        flag = true;
        length++;
      } else if (Block::verify(blk, lastBlock.value())) {
        this->lastBlock = blk;
        length++;
      } else {
        db.remove<TxEntity>(
            sqlite_orm::where(sqlite_orm::c(&TxEntity::blockId) == blks[i].id));
        db.remove<BlockEntity>(blks[i].id);
      }
    }
    return flag;
  }
  std::vector<Tx> getTxsByBlkId(int id) {
    std::vector<Tx> txs;
    std::vector<TxEntity> txes = db.get_all<TxEntity>(
        sqlite_orm::where(sqlite_orm::c(&TxEntity::blockId) == id));

    for (int i = 0; i < txes.size(); i++) {
      txs.push_back(Tx::createTx(&txes[i]));
    }

    return txs;
  }

public:
  Blockchain(std::string db_name, uint diff = 1) : db{createDB(db_name)} {
    db.sync_schema();

    if (!scanDB()) {
      auto blk = Block::createGenesis();
      addBlock(blk);
    }

    this->diff = diff;
  };

  std::string getlastHash() { return lastBlock->getHash(); }
  uint64_t getLastIndex() { return lastBlock->getIndex() + 1; }

  ChainStatus addBlock(Block &blk) {
    bool flag = length == 0 ? true : Block::verify(blk, lastBlock.value());

    if (flag) {
      if (this->addToDB(&blk)) {
        this->lastBlock = blk;
        length++;
        return Ok;
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
      auto txs = getTxsByBlkId(blks[i].id);
      auto blk = Block::createBlock(&blks[i], std::move(txs));
      blk.print();
    }
  }
};
} // namespace Mirucoin
#endif