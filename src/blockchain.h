#ifndef BLOCKCHAIN_LIB
#define BLOCKCHAIN_LIB

#include "./block.h"
#include "./mirucoin.h"
#include "./tx.h"
#include "ledger.h"
#include "mirucoin.h"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
#include <sqlite_orm/sqlite_orm.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Mirucoin {
#define STORAGE_START_BALLANCE 1000000000
#define MAX_BLOCK_SIZE 10 * 1024 * 1024
#define STORAGE_ADDR "Null::"

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
          sqlite_orm::make_column("diff", &BlockEntity::diff),
          sqlite_orm::make_column("timestamp", &BlockEntity::timestamp)),
      sqlite_orm::make_table(
          "Txs",
          sqlite_orm::make_column("id", &TxEntity::id,
                                  sqlite_orm::primary_key().autoincrement()),
          sqlite_orm::make_column("payer", &TxEntity::payer),
          sqlite_orm::make_column("payee", &TxEntity::payee),
          sqlite_orm::make_column("amount", &TxEntity::amount),
          sqlite_orm::make_column("fee", &TxEntity::fee),
          sqlite_orm::make_column("gasprice", &TxEntity::gasprice),
          sqlite_orm::make_column("data", &TxEntity::data),
          sqlite_orm::make_column("timestamap", &TxEntity::timestamp),
          sqlite_orm::make_column("hash", &TxEntity::hash),
          sqlite_orm::make_column("sign", &TxEntity::sign),
          sqlite_orm::make_column("seed", &TxEntity::seed),
          sqlite_orm::make_column("blockId", &TxEntity::blockId),
          sqlite_orm::foreign_key(&TxEntity::blockId)
              .references(&BlockEntity::id)));
}

class Blockchain {
private:
  LedgerEntity storage{STORAGE_ADDR, STORAGE_START_BALLANCE};
  std::unordered_map<std::string, LedgerEntity> wallets;
  std::unordered_map<std::string, Tx> txspool;
  std::optional<Block> lastBlock;
  uint64_t length = 0;
  uint diff;
  Status status = Ok;
  float miner_reward = 100;
  decltype(createDB(std::declval<std::string>())) db;
  Status addToDB(Block *blk) {
    try {
      db.begin_transaction();
      BlockEntity block{-1,
                        blk->getMerkleRoot(),
                        blk->getHash(),
                        blk->getPrevHash(),
                        blk->getNonce(),
                        blk->getIndex(),
                        diff,
                        blk->getTimestamp()};
      block.id = db.insert(block);

      auto &txs = blk->getData();

      for (int i = 0; i < txs.size(); i++) {
        db.insert(TxEntity{
            -1, txs[i].getPayer(), txs[i].getPayee(), txs[i].getAmount(),
            txs[i].getFee(), txs[i].getGasPrice(), txs[i].getData(),
            txs[i].getTimestamp(), txs[i].getHash(), txs[i].getSign(),
            txs[i].getSeed(), std::make_unique<int>(block.id)});
        updateWallet(txs[i], 1);
      }
      db.commit();
    } catch (...) {
      db.rollback();
      std::cout << "DB Error" << std::endl;
      return DB_Error;
    }
    return Ok;
  }

  void updateWallets() {
    auto txs = lastBlock->getData();
    for (int i = 0; i < txs.size(); i++) {
      if (wallets.contains(txs[i].getPayee())) {
        wallets[txs[i].getPayee()].ballance += txs[i].getAmount();
      } else {
        LedgerEntity w{txs[i].getPayee(), txs[i].getAmount()};
        wallets.insert({txs[i].getPayee(), w});
      }
      if (wallets.contains(txs[i].getPayer())) {
        wallets[txs[i].getPayer()].ballance -=
            txs[i].getAmount() - txs[i].getFee();
      } else {
        LedgerEntity w{txs[i].getPayer(),
                       -txs[i].getAmount() - txs[i].getFee()};
        wallets.insert({txs[i].getPayer(), w});
      }
      wallets[storage.address].ballance += txs[i].getFee();
    }
  }

  // flag = 0 - add to mempool, 1 - add to chain, 2 - delete from chain
  void updateWallets(std::vector<Tx> &txs, uint8_t flag = 0) {
    for (int i = 0; i < txs.size(); i++) {
      updateWallet(txs[i], flag);
    }
  }

  void updateWallet(Tx tx, uint8_t flag = 0) {
    {
      float li = 0;
      float b = 0;
      if (flag == 0) {
        li += tx.getAmount();
      } else if (flag == 1) {
        b += tx.getAmount();
        li -= tx.getAmount();
      } else if (flag == 2) {
        li -= tx.getAmount();
      }
      if (wallets.contains(tx.getPayee())) {
        wallets[tx.getPayee()].ballance += b;
        wallets[tx.getPayee()].lockedIn += li;
      } else {
        LedgerEntity w{tx.getPayee(), b, 0, li};
        wallets.insert({tx.getPayee(), w});
      }
    }
    {
      float lo = 0;
      float b = 0;
      auto l = tx.getAmount() + tx.getFee();
      if (flag == 0) {
        lo += l;
        b -= l;
      } else if (flag == 1) {
        lo -= l;
      } else if (flag == 2) {
        lo -= l;
        b += l;
      }
      if (wallets.contains(tx.getPayer())) {
        wallets[tx.getPayer()].ballance += b;
        wallets[tx.getPayer()].lockedOut += lo;
      } else {
        LedgerEntity w{tx.getPayer(), b, lo, 0};
        wallets.insert({tx.getPayer(), w});
      }
    }
    if (flag == 1)
      wallets[storage.address].ballance += tx.getFee();
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
        updateWallets();
      } else if (Block::verify(blk, lastBlock.value())) {
        this->lastBlock = blk;
        length++;
        updateWallets();
      } else {
        db.remove_all<TxEntity>(
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
  Status addBlock(Block &blk) {
    bool flag = length == 0 ? true : Block::verify(blk, lastBlock.value());

    if (blk.getDifficulty() < 1 && Block::chekHash(blk))
      return VerificationError;

    if (flag) {
      auto r = this->addToDB(&blk);
      if (r == Ok) {
        this->lastBlock = blk;
        length++;
        for (int i = 0; i < this->lastBlock.value().getData().size(); i++) {
          txspool.erase(this->lastBlock.value().getData()[i].getHash());
        }
        // updateWallets();
        return Ok;
      } else {
        return InsertError;
      }
    } else {
      return VerificationError;
    }
  }
  bool addTx(Tx &tx) {
    if (!wallets.contains(tx.getPayer()) ||
        (wallets[tx.getPayer()].ballance < (tx.getAmount() + tx.getFee()))) {
      return false;
    }

    if (txspool.contains(tx.getHash()))
      return true;

    auto tt = db.get_all<TxEntity>(
        sqlite_orm::where(sqlite_orm::c(&TxEntity::hash) == tx.getHash()));

    if (tt.size() > 1)
      return true;

    txspool.insert({tx.getHash(), tx});
    updateWallet(tx);
    return true;
  }
  static bool cmpTxs(Tx &a, Tx &b) {
    if (a.getPayer() == STORAGE_ADDR)
      return true;
    else
      return a.getFee() > b.getFee();
  }

public:
  Blockchain(std::string db_name, uint diff = 1) : db{createDB(db_name)} {
    db.sync_schema();

    wallets.insert({storage.address, storage});

    this->diff = diff;

    if (!scanDB()) {
      auto blk = Block::createGenesis(diff);
      addBlock(blk);
    }
  };

  LedgerEntity getLedger(std::string addr) {
    if (!wallets.contains(addr)) {
      return LedgerEntity();
    }
    return wallets[addr];
  }

  void createTx(std::string b) {
    Tx tx{storage.address, b, 10, 1, ""};
    addTx(tx);
  }

  std::string getlastHash() { return lastBlock->getHash(); }
  uint64_t getLastIndex() { return lastBlock->getIndex() + 1; }

  Block getNewBlock() {

    std::vector<Tx> tmp;

    for (auto &e : txspool) {
      tmp.push_back(e.second);
    }

    std::sort(tmp.rbegin(), tmp.rend(), cmpTxs);

    int size = 0;

    std::vector<Tx> accept;

    // std::cout << tmp.size() << std::endl;
    // std::cout << accept.size() << std::endl;
    while (size < MAX_BLOCK_SIZE && !tmp.empty()) {
      size += tmp.back().size();
      accept.push_back(tmp.back());
      tmp.pop_back();
    }
    // std::cout << tmp.size() << std::endl;
    // std::cout << accept.size() << std::endl;

    return Block{lastBlock->getHash(), length, accept, diff};
  }
  Status addBlock(Block &blk, std::string miner) {
    auto r = addBlock(blk);
    if (r == Ok && !miner.empty()) {
      Tx tx{storage.address, miner, miner_reward, 0, "REWARD"};
      addTx(tx);
    }
    return r;
  }

  bool addTx(Tx &tx, std::string pubkey) {
    if (!wallets.contains(tx.getPayer()) ||
        (wallets[tx.getPayer()].ballance < (tx.getAmount() + tx.getFee()))) {
      return false;
    }

    if (txspool.contains(tx.getHash()))
      return true;

    auto tt = db.get_all<TxEntity>(
        sqlite_orm::where(sqlite_orm::c(&TxEntity::hash) == tx.getHash()));

    if (tt.size() > 1)
      return true;

    if (Ledger::verify(tx.getHash(), tx.getSign(), pubkey)) {
      txspool.insert({tx.getHash(), tx});
      updateWallet(tx, 0);
      return true;
    }
    return false;
  }

  int getStatus() { return status; }

  void printChain() {
    std::vector<BlockEntity> blks = db.get_all<BlockEntity>();

    printf("Chain length: %lu\n", blks.size());

    for (int i = 0; i < blks.size(); i++) {
      auto txs = getTxsByBlkId(blks[i].id);
      auto blk = Block::createBlock(&blks[i], txs);
      blk.print();
      printTxs(txs);
    }
    printf("Mempool size: %zu\n", txspool.size());
  }
  void printTxs(std::vector<Tx> &txs) {

    printf("Txs count: %lu\n", txs.size());

    for (int i = 0; i < txs.size(); i++) {
      txs[i].print();
    }
  }
  void printTxs(int blkId) {
    std::vector<TxEntity> txs = db.get_all<TxEntity>(
        sqlite_orm::where(sqlite_orm::c(&TxEntity::blockId) == blkId));

    printf("Txs count: %lu\n", txs.size());

    for (int i = 0; i < txs.size(); i++) {
      auto tx = Tx::createTx(&txs[i]);
      tx.print();
    }
  }
};
} // namespace Mirucoin
#endif
