#include "./blockchain.h"
#include "./ledger.h"
#include "./miner.h"
#include "block.h"
#include <string>

using namespace Mirucoin;
using namespace std;

int main(int, char **) {
  std::srand(std::time(nullptr));
  // Mirucoin::Blockchain chain{(std::string) "chain.sqlite"};

  // Mirucoin::Tx tx = Mirucoin::Tx("me", "alice", 12, 0.1, "");
  // std::vector<Mirucoin::Tx> txs;
  // txs.push_back(std::move(tx));

  // Mirucoin::Block blk{chain.getlastHash(), chain.getLastIndex(),
  //                     std::move(txs)};

  // chain.addBlock(blk);

  // chain.print();

  // Mirucoin::Ledger wallet;

  // std::string message = "Some message";

  // printf("PrivateKey: %s\nPublicKey: %s\nAddress: %s\n",
  //        wallet.getDerPrivate().c_str(), wallet.getDerPublic().c_str(),
  //        wallet.getAddress().c_str());

  // std::string sing = wallet.sign(message);

  // std::cout << sing << std::endl;
  // std::cout << wallet.verify(message, sing, wallet.getDerPublic()) <<
  // std::endl;

  Blockchain chain{"chain.sqlite", 3};

  Ledger walletA;
  Ledger walletB;

  Miner miner{walletA.getAddress()};

  chain.createTx(walletA.getAddress());

  Block blk = chain.getNewBlock();
  blk = miner.mine(blk);
  chain.addBlock(blk, walletA.getAddress());

  walletA.update(chain.getLedger(walletA.getAddress()));
  walletB.update(chain.getLedger(walletB.getAddress()));

  walletA.print();

  auto tx = walletA.send(walletB.getAddress(), 5, 0.1);

  if (tx.has_value()) {
    chain.addTx(tx.value(), walletA.getDerPublic());
    Block blk = chain.getNewBlock();
    blk = miner.mine(blk);
    chain.addBlock(blk, walletA.getAddress());
  }

  walletA.update(chain.getLedger(walletA.getAddress()));
  walletB.update(chain.getLedger(walletB.getAddress()));

  walletA.print();
  walletB.print();

  for (int i = 0; i < 5; i++) {
    Block blk = chain.getNewBlock();
    blk = miner.mine(blk);
    chain.addBlock(blk, walletA.getAddress());
  }

  walletA.update(chain.getLedger(walletA.getAddress()));
  walletB.update(chain.getLedger(walletB.getAddress()));

  walletA.print();

  chain.printChain();

  return 0;
}