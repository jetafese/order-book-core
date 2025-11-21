#include "vOfferExchange.h"

using namespace stellar;

int main() {
    // auto res = exchangeV10(Price{3, 2}, 28, 27, INT64_MAX, INT64_MAX,
    //                         RoundingType::NORMAL);
    // assert(res.wheatStays == (28 > 27));
    // assert(res.numWheatReceived == 0);
    // assert(res.numSheepSend == 0);
    // // assert(false);
    // return res.numWheatReceived;
    int32_t a = sea_nd_i32();
    assume(a >= 0);
    assert(a <= INT32_MAX);
    return 0;
}