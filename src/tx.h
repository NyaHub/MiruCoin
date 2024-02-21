#ifndef TX_LIB
#define TX_LIB

#include "./utils.h"
#include "json.hpp"
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <string>

namespace Mirucoin {
struct TxEntity {
  int id;
  std::string payer;
  std::string payee;
  float amount;
  float fee;
  std::string data;
  time_t timestamp;
  std::string hash;
  std::string sign;
  int seed;
  std::unique_ptr<int> blockId;
};

class Tx {
private:
  std::string payer;
  std::string payee;
  float amount;
  float fee;
  std::string data;
  time_t timestamp;
  nlohmann::json header;
  std::string hash;
  std::string sign;
  int seed;

public:
  Tx() {
    this->payer = "";
    this->payee = "";
    this->amount = 0;
    this->fee = 0;
    this->data = "";
    this->timestamp = 0;
    this->sign = "";
    this->seed = std::rand();

    this->updateHeader("", "");
  }
  Tx(Tx const &a) {
    this->payer = a.payer;
    this->payee = a.payee;
    this->amount = a.amount;
    this->fee = a.fee;
    this->data = a.data;
    this->timestamp = a.timestamp;
    this->sign = a.sign;
    this->seed = a.seed;

    this->updateHeader("", "");
  }
  Tx(std::string payer, std::string payee, float amount, float fee,
     std::string data, int seed = -1) {
    this->payer = payer;
    this->payee = payee;
    this->amount = amount;
    this->fee = fee;
    this->data = data;
    this->timestamp = std::time(nullptr);
    this->sign = "";
    if (seed == -1) {
      this->seed = std::rand();
    } else {
      this->seed = seed;
    }

    this->updateHeader("", "");
  };
  void setSign(std::string sign) { this->sign = sign; }
  void updateHeader(std::string key = "", std::string val = "") {
    if ((key.empty() || val.empty()) && !this->hash.empty()) {
      return;
    }
    if (key == "payer") {
      this->payer = val;
    } else if (key == "payee") {
      this->payee = val;
    } else if (key == "data") {
      this->data = val;
    }
    this->header = {
        {"payer", this->payer},         {"payee", this->payee},
        {"amount", this->amount},       {"fee", this->fee},
        {"timestamp", this->timestamp}, {"data", sha256String(this->data)},
        {"seed", this->seed},
    };

    this->hash = sha256String(this->header.dump());
  }
  void updateHeader(std::string key = "", int64_t val = 0) {
    if ((key.empty() || !val) && !this->hash.empty()) {
      return;
    }
    if (key == "amount") {
      this->amount = val;
    } else if (key == "timestamp") {
      this->timestamp = val;
    }
    this->header = {
        {"payer", this->payer},         {"payee", this->payee},
        {"amount", this->amount},       {"fee", this->fee},
        {"timestamp", this->timestamp}, {"data", sha256String(this->data)},
        {"seed", this->seed},
    };

    this->hash = sha256String(this->header.dump());
  }
  void updateHeader(std::string key = "", float val = 0) {
    if ((key.empty() || !val) && !this->hash.empty()) {
      return;
    }
    if (key == "fee") {
      this->fee = val;
    }
    this->header = {
        {"payer", this->payer},         {"payee", this->payee},
        {"amount", this->amount},       {"fee", this->fee},
        {"timestamp", this->timestamp}, {"data", sha256String(this->data)},
        {"seed", this->seed},
    };

    this->hash = sha256String(this->header.dump());
  }
  const std::string &getPayer() const { return this->payer; }
  const std::string &getPayee() const { return this->payee; }
  const float getAmount() const { return this->amount; }
  const float getFee() const { return this->fee; }
  const std::string &getData() const { return this->data; }
  const time_t getTimestamp() const { return this->timestamp; }
  const std::string &getHash() const { return this->hash; }
  const std::string &getSign() const { return this->sign; }
  const int getSeed() const { return this->seed; }

  nlohmann::json toJson() {
    nlohmann::json tx = {{"header", header}, {"data", data}};
    return tx;
  }

  std::string toString() { return this->toJson().dump(); }

  static Tx createTx(TxEntity *txe) {
    Tx tx =
        Tx(txe->payer, txe->payee, txe->amount, txe->fee, txe->data, txe->seed);
    tx.updateHeader("timestamp", txe->timestamp);
    tx.setSign(txe->sign);
    return tx;
  }
  void print() {
    printf("Tx: %s\n\tPayer: %s\n\tPayee: %s\n\tAmount: %f\n\tFee: "
           "%f\n\tTimestamp: %lu\n\tSign: %s\n\tSeed: %i\n\tData: %s\n",
           hash.c_str(), payer.c_str(), payee.c_str(), amount, fee, timestamp,
           sign.c_str(), seed, data.c_str());
  }
};

} // namespace Mirucoin

#endif