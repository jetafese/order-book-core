// Test drivers adapted from stellar-core/src/transactions/test for OfferExchange translation unit

#include <cassert>
#include "OfferExchange.h"

using namespace stellar;

void testRoundingForPATH_PAYMENT_STRICT_RECEIVE();
void testRoundingForPATH_PAYMENT_STRICT_SEND();
void testLimitedByMaxWheatSendAndMaxSheepSend();
void testLimitedByMaxWheatReceiveAndMaxSheepReceive();
void testLimitedByMaxWheatSendAndMaxWheatReceive();
void testLimitedByMaxSheepSendAndMaxSheepReceive();
void testThreshold();

int main()
{
    testRoundingForPATH_PAYMENT_STRICT_RECEIVE();
    testRoundingForPATH_PAYMENT_STRICT_SEND();
    testLimitedByMaxWheatSendAndMaxSheepSend();
    testLimitedByMaxWheatReceiveAndMaxSheepReceive();
    testLimitedByMaxWheatSendAndMaxWheatReceive();
    testLimitedByMaxSheepSendAndMaxSheepReceive();
    testThreshold();
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

// SECTION("Rounding for PATH_PAYMENT_STRICT_SEND")
void testRoundingForPATH_PAYMENT_STRICT_SEND() {
    auto check = [](Price const& p, int64_t maxWheatSend,
                    int64_t maxWheatReceive, int64_t maxSheepSend,
                    RoundingType round, int64_t wheatReceive,
                    int64_t sheepSend) {
        auto res = exchangeV10(p, maxWheatSend, maxWheatReceive,
                                maxSheepSend, INT64_MAX, round);
        // This is not generally true, but it is a simple interface for what
        // we need to test.
        if (maxWheatReceive == INT64_MAX)
        {
            assert(res.wheatStays);
        }
        else
        {
            assert(res.wheatStays ==
                    bigMultiply(maxWheatSend, p.n) >
                        std::min(bigMultiply(maxSheepSend, p.d),
                                    bigMultiply(maxWheatReceive, p.n)));
        }
        assert(res.numWheatReceived == wheatReceive);
        assert(res.numSheepSend == sheepSend);
    };

    // SECTION("no thresholding")
    check(Price{3, 2}, 28, INT64_MAX, 41, RoundingType::NORMAL, 0, 0);
    check(Price{3, 2}, 28, INT64_MAX, 41, RoundingType::PATH_PAYMENT_STRICT_SEND, 27, 41);

    // // SECTION("transfer can increase if wheat is more valuable")
    // assert(adjustOffer(Price{3, 2}, 97, INT64_MAX) == 97);
    // check(Price{3, 2}, 97, INT64_MAX, 145, RoundingType::NORMAL, 96, 144);
    // check(Price{3, 2}, 97, INT64_MAX, 145, RoundingType::PATH_PAYMENT_STRICT_SEND, 96, 145);

    // SECTION("transfer can increase if sheep is more valuable")
    check(Price{2, 3}, 97, 95, INT64_MAX, RoundingType::NORMAL, 94, 63);
    check(Price{2, 3}, 97, 95, INT64_MAX, RoundingType::PATH_PAYMENT_STRICT_SEND, 95, INT64_MAX);

    // SECTION("can send nonzero while receiving zero")
    check(Price{2, 1}, 1, INT64_MAX, 1, RoundingType::NORMAL, 0, 0);
    check(Price{2, 1}, 1, INT64_MAX, 1, RoundingType::PATH_PAYMENT_STRICT_SEND, 0, 1);
}

// SECTION("Limited by maxWheatSend and maxSheepSend")
void testLimitedByMaxWheatSendAndMaxSheepSend() {
    auto checkExchangeV10 = [](Price const& p, int64_t maxWheatSend,
                                int64_t maxSheepSend, int64_t wheatReceive,
                                int64_t sheepSend) {
        auto res = exchangeV10(p, maxWheatSend, INT64_MAX, maxSheepSend,
                                INT64_MAX, RoundingType::NORMAL);
        assert(res.wheatStays ==
                (maxWheatSend * p.n > maxSheepSend * p.d));
        assert(res.numWheatReceived == wheatReceive);
        assert(res.numSheepSend == sheepSend);
        if (res.wheatStays)
        {
            assert(sheepSend * p.d >= wheatReceive * p.n);
        }
        else
        {
            assert(sheepSend * p.d <= wheatReceive * p.n);
        }
    };

    // SECTION("price > 1")
    // Exact boundary
    checkExchangeV10(Price{3, 2}, 3000, 4501, 3000, 4500);
    checkExchangeV10(Price{3, 2}, 3000, 4500, 3000, 4500);
    checkExchangeV10(Price{3, 2}, 3000, 4499, 2999, 4499);

    // Boundary between two values
    checkExchangeV10(Price{3, 2}, 2999, 4499, 2999, 4498);
    checkExchangeV10(Price{3, 2}, 2999, 4498, 2998, 4497);

    // SECTION("price < 1")
    // Exact boundary
    checkExchangeV10(Price{2, 3}, 3000, 2001, 3000, 2000);
    checkExchangeV10(Price{2, 3}, 3000, 2000, 3000, 2000);
    checkExchangeV10(Price{2, 3}, 3000, 1999, 2998, 1999);

    // Boundary between two values
    checkExchangeV10(Price{2, 3}, 2999, 2000, 2999, 1999);
    checkExchangeV10(Price{2, 3}, 2999, 1999, 2998, 1999);
}

// SECTION("Limited by maxWheatReceive and maxSheepReceive")
void testLimitedByMaxWheatReceiveAndMaxSheepReceive() {
    auto checkExchangeV10 = [](Price const& p, int64_t maxWheatReceive,
                                int64_t maxSheepReceive,
                                int64_t wheatReceive, int64_t sheepSend) {
        auto res = exchangeV10(p, INT64_MAX, maxWheatReceive, INT64_MAX,
                                maxSheepReceive, RoundingType::NORMAL);
        assert(res.wheatStays ==
                (maxSheepReceive * p.d > maxWheatReceive * p.n));
        assert(res.numWheatReceived == wheatReceive);
        assert(res.numSheepSend == sheepSend);
        if (res.wheatStays)
        {
            assert(sheepSend * p.d >= wheatReceive * p.n);
        }
        else
        {
            assert(sheepSend * p.d <= wheatReceive * p.n);
        }
    };

    // SECTION("price > 1")
    // Exact boundary
    checkExchangeV10(Price{3, 2}, 3000, 4501, 3000, 4500);
    checkExchangeV10(Price{3, 2}, 3000, 4500, 3000, 4500);
    checkExchangeV10(Price{3, 2}, 3000, 4499, 2999, 4498);

    // Boundary between two values
    checkExchangeV10(Price{3, 2}, 2999, 4499, 2999, 4499);
    checkExchangeV10(Price{3, 2}, 2999, 4498, 2998, 4497);

    // SECTION("price < 1")
    // Exact boundary
    checkExchangeV10(Price{2, 3}, 3000, 2001, 3000, 2000);
    checkExchangeV10(Price{2, 3}, 3000, 2000, 3000, 2000);
    checkExchangeV10(Price{2, 3}, 3000, 1999, 2999, 1999);

    // Boundary between two values
    checkExchangeV10(Price{2, 3}, 2999, 2000, 2998, 1999);
    checkExchangeV10(Price{2, 3}, 2999, 1999, 2999, 1999);
}

// SECTION("Limited by maxWheatSend and maxWheatReceive")
void testLimitedByMaxWheatSendAndMaxWheatReceive() {
    auto checkExchangeV10 = [](Price const& p, int64_t maxWheatSend,
                                int64_t maxWheatReceive,
                                int64_t wheatReceive, int64_t sheepSend) {
        auto res = exchangeV10(p, maxWheatSend, maxWheatReceive, INT64_MAX,
                                INT64_MAX, RoundingType::NORMAL);
        assert(res.wheatStays == (maxWheatSend > maxWheatReceive));
        assert(res.numWheatReceived == wheatReceive);
        assert(res.numSheepSend == sheepSend);
        if (res.wheatStays)
        {
            assert(sheepSend * p.d >= wheatReceive * p.n);
        }
        else
        {
            assert(sheepSend * p.d <= wheatReceive * p.n);
        }
    };

    // SECTION("price > 1")
    // Exact boundary (boundary between values impossible in this case)
    checkExchangeV10(Price{3, 2}, 3000, 3001, 3000, 4500);
    checkExchangeV10(Price{3, 2}, 3000, 3000, 3000, 4500);
    checkExchangeV10(Price{3, 2}, 3000, 2999, 2999, 4499);

    // SECTION("price < 1")
    // Exact boundary (boundary between values impossible in this case)
    checkExchangeV10(Price{2, 3}, 3000, 3001, 3000, 2000);
    checkExchangeV10(Price{2, 3}, 3000, 3000, 3000, 2000);
    checkExchangeV10(Price{2, 3}, 3000, 2999, 2998, 1999);
}

// SECTION("Limited by maxSheepSend and maxSheepReceive")
void testLimitedByMaxSheepSendAndMaxSheepReceive() {
    auto checkExchangeV10 = [](Price const& p, int64_t maxSheepSend,
                                int64_t maxSheepReceive,
                                int64_t wheatReceive, int64_t sheepSend) {
        auto res = exchangeV10(p, INT64_MAX, INT64_MAX, maxSheepSend,
                                maxSheepReceive, RoundingType::NORMAL);
        assert(res.wheatStays == (maxSheepReceive > maxSheepSend));
        assert(res.numWheatReceived == wheatReceive);
        assert(res.numSheepSend == sheepSend);
        if (res.wheatStays)
        {
            assert(sheepSend * p.d >= wheatReceive * p.n);
        }
        else
        {
            assert(sheepSend * p.d <= wheatReceive * p.n);
        }
    };

    // SECTION("price > 1")
    // Exact boundary (boundary between values impossible in this case)
    checkExchangeV10(Price{3, 2}, 4500, 4501, 3000, 4500);
    checkExchangeV10(Price{3, 2}, 4500, 4500, 3000, 4500);
    checkExchangeV10(Price{3, 2}, 4500, 4499, 2999, 4498);

    // SECTION("price < 1")
    // Exact boundary (boundary between values impossible in this case)
    checkExchangeV10(Price{2, 3}, 2000, 2001, 3000, 2000);
    checkExchangeV10(Price{2, 3}, 2000, 2000, 3000, 2000);
    checkExchangeV10(Price{2, 3}, 2000, 1999, 2999, 1999);
}

// SECTION("Threshold")
void testThreshold() {
    auto checkExchangeV10 = [](Price const& p, int64_t maxWheatSend,
                                int64_t maxWheatReceive,
                                int64_t wheatReceive, int64_t sheepSend) {
        auto res = exchangeV10(p, maxWheatSend, maxWheatReceive, INT64_MAX,
                                INT64_MAX, RoundingType::NORMAL);
        assert(res.wheatStays == (maxWheatSend > maxWheatReceive));
        assert(res.numWheatReceived == wheatReceive);
        assert(res.numSheepSend == sheepSend);
        if (res.wheatStays)
        {
            assert(sheepSend * p.d >= wheatReceive * p.n);
        }
        else
        {
            assert(sheepSend * p.d <= wheatReceive * p.n);
        }
    };

    // Exchange nothing if thresholds exceeded
    checkExchangeV10(Price{3, 2}, 28, 27, 0, 0);
    checkExchangeV10(Price{3, 2}, 28, 26, 26, 39);

    // Thresholds not exceeded for sufficiently large offers
    checkExchangeV10(Price{3, 2}, 52, 51, 51, 77);
    checkExchangeV10(Price{3, 2}, 52, 50, 50, 75);
}

