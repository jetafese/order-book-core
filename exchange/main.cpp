// Test drivers adapted from stellar-core/src/transactions/test for OfferExchange translation unit

#include <cassert>
#include "OfferExchange.h"

using namespace stellar;

void testRoundingForPATH_PAYMENT_STRICT_RECEIVE();

int main()
{
    testRoundingForPATH_PAYMENT_STRICT_RECEIVE();
    return 0;
}

// SECTION("Rounding for PATH_PAYMENT_STRICT_RECEIVE")
void testRoundingForPATH_PAYMENT_STRICT_RECEIVE() {
    auto check = [](Price const& p, int64_t maxWheatSend,
                    int64_t maxWheatReceive, RoundingType round,
                    int64_t wheatReceive, int64_t sheepSend) {
        auto res = exchangeV10(p, maxWheatSend, maxWheatReceive, INT64_MAX,
                                INT64_MAX, round);
        assert(res.wheatStays == (maxWheatSend > maxWheatReceive));
        assert(res.numWheatReceived == wheatReceive);
        assert(res.numSheepSend == sheepSend);
    };

    // SECTION("no thresholding")
    check(Price{3, 2}, 28, 27, RoundingType::NORMAL, 0, 0);
    check(Price{3, 2}, 28, 27,
            RoundingType::PATH_PAYMENT_STRICT_RECEIVE, 27, 41);

    // SECTION("result is unchanged if wheat is more valuable")
    check(Price{3, 2}, 150, 101, RoundingType::NORMAL, 101, 152);
    check(Price{3, 2}, 150, 101, RoundingType::PATH_PAYMENT_STRICT_RECEIVE, 101, 152);

    // SECTION("transfer can increase if sheep is more valuable")
    check(Price{2, 3}, 150, 101, RoundingType::NORMAL, 100, 67);
    check(Price{2, 3}, 150, 101, RoundingType::PATH_PAYMENT_STRICT_RECEIVE, 101, 68);
}
