#include "./block.h"
#include <memory>
#include <sqlite_orm/sqlite_orm.h>
#include <string>
#include <utility>

namespace Mirucoin {
enum ChainStatus {
  Ok,
  DB_Error, // can't open database
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
  uint diff;
  ChainStatus status = Ok;
  decltype(createDB(std::declval<std::string>())) db;

public:
  Blockchain(std::string db_name, uint diff = 1) : db{createDB(db_name)} {
    lastBlock = Block::createGenesis();
    this->diff = diff;

    db.sync_schema();
  };
  int getStatus() { return status; }
};
} // namespace Mirucoin
