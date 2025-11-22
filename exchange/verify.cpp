#include "vOfferExchange.h"

using namespace stellar;

RoundingType nd_roundingType() {
    int32_t val = sea_nd_i32();
    if (val < 0) return RoundingType::NORMAL;
    if (val == 0) return RoundingType::PATH_PAYMENT_STRICT_SEND;
    return RoundingType::PATH_PAYMENT_STRICT_RECEIVE;
}

void verify_wheatStays_PATH_PAYMENT_STRICT_SEND();
void verify_wheatStays_nztrade_moreSheep_than_wheat();
void verify_not_wheatStays_nztrade_lessSheep_than_wheat();

int main() {
    verify_wheatStays_PATH_PAYMENT_STRICT_SEND();
    verify_wheatStays_nztrade_moreSheep_than_wheat();
    verify_not_wheatStays_nztrade_lessSheep_than_wheat();
    return 0;
}

// we restrict the inputs to ensure wheatStays is true
// round is set to PATH_PAYMENT_STRICT_SEND
// we expect that wheatStays => sheepSend > 0 and wheatReceived >= 0
// WILL PANIC if we call exchangeV10(...) instead
void verify_wheatStays_PATH_PAYMENT_STRICT_SEND() {
    // invariants: established by callers
    int32_t n = sea_nd_i32();
    int32_t d = sea_nd_i32();
    assume(n > 0);
    assume(d > 0);

    int64_t maxWheatSend = sea_nd_i64();
    int64_t maxWheatReceive = sea_nd_i64();
    int64_t maxSheepSend = sea_nd_i64();
    int64_t maxSheepReceive = sea_nd_i64();
    assume(maxWheatSend >= 0);
    assume(maxWheatReceive > 0);
    assume(maxSheepSend > 0);
    RoundingType round = RoundingType::PATH_PAYMENT_STRICT_SEND;
    // invariants: this rounding type requires min(maxSheepSend, maxSheepReceive) > 0
    assume(maxSheepReceive > 0);
    
    Price p{n, d};
    auto res = exchangeV10WithoutPriceErrorThresholds(p, maxWheatSend, maxWheatReceive, maxSheepSend, maxSheepReceive, round);
    
    assert(!res.wheatStays || (res.numSheepSend > 0 && res.numWheatReceived >= 0));
}

// we restrict the inputs to ensure wheatStays is true
// TODO: there is a bug with PATH_PAYMENT_STRICT_SEND case so we restrict round to other two types
// we expect that wheatStays and non-zero trade => price.d * sheepSend >= price.n * wheatReceived
void verify_wheatStays_nztrade_moreSheep_than_wheat() {
    // invariants: established by callers
    int32_t n = sea_nd_i32();
    int32_t d = sea_nd_i32();
    assume(n > 0);
    assume(d > 0);
    int64_t maxWheatSend = sea_nd_i64();
    int64_t maxWheatReceive = sea_nd_i64();
    int64_t maxSheepSend = sea_nd_i64();
    int64_t maxSheepReceive = sea_nd_i64();
    assume(maxWheatSend >= 0);
    assume(maxWheatReceive > 0);
    assume(maxSheepSend > 0);
    assume(maxSheepReceive >= 0);

    RoundingType round = nd_roundingType();
    assume(round != RoundingType::PATH_PAYMENT_STRICT_SEND); // avoid the case we already verified
    Price p{n, d};
    auto res = exchangeV10(p, maxWheatSend, maxWheatReceive, maxSheepSend, maxSheepReceive, round);
    
    uint128_t wheatReceiveValue = bigMultiply(res.numWheatReceived, p.n);
    uint128_t sheepSendValue = bigMultiply(res.numSheepSend, p.d);
    assert(!(res.wheatStays && res.numWheatReceived > 0 && res.numSheepSend > 0) ||
           (sheepSendValue >= wheatReceiveValue));
}

// we restrict the inputs to ensure wheatStays is false
// TODO: there is a bug with PATH_PAYMENT_STRICT_SEND case so we restrict round to other two types
// we expect that not wheatStays and non-zero trade => price.d * sheepSend <= price.n * wheatReceived
void verify_not_wheatStays_nztrade_lessSheep_than_wheat() {
    // invariants established by callers
    int32_t n = sea_nd_i32();
    int32_t d = sea_nd_i32();
    assume(n > 0);
    assume(d > 0);
    int64_t maxWheatSend = sea_nd_i64();
    int64_t maxWheatReceive = sea_nd_i64();
    int64_t maxSheepSend = sea_nd_i64();
    int64_t maxSheepReceive = sea_nd_i64();
    assume(maxWheatSend >= 0);
    assume(maxWheatReceive > 0);
    assume(maxSheepSend > 0);
    assume(maxSheepReceive >= 0);

    RoundingType round = nd_roundingType();
    assume(round != RoundingType::PATH_PAYMENT_STRICT_SEND); // avoid the case we already verified
    Price p{n, d};
    auto res = exchangeV10(p, maxWheatSend, maxWheatReceive, maxSheepSend, maxSheepReceive, round);
    
    uint128_t wheatReceiveValue = bigMultiply(res.numWheatReceived, p.n);
    uint128_t sheepSendValue = bigMultiply(res.numSheepSend, p.d);
    assert(!(!res.wheatStays && res.numWheatReceived > 0 && res.numSheepSend > 0) ||
           (sheepSendValue <= wheatReceiveValue));
}
