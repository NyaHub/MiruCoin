#ifndef MIRUCOIN_H
#define MIRUCOIN_H

namespace Mirucoin {
enum Status {
  Ok,
  DB_Error, // can't open database
  VerificationError,
  InsertError,
  NotEnoughMoney,
};
}

#endif