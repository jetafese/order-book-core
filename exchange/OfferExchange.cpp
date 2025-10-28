// Fork of Stellar Development Foundation and contributors: stellar-core/src/transactions/OfferExchange.cpp

#include "OfferExchange.h"

struct ExchangedQuantities
{
    int64_t sheepSend{0};
    int64_t wheatReceived{0};
};

namespace stellar {

void printAssertFailureAndAbort(const char* s1, const char* file, int line)
{
    std::fprintf(stderr, "%s at %s:%d\n", s1, file, line);
    std::fflush(stderr);
    // printCurrentBacktrace();
    // dbgAbort();
    std::abort();
}

void printAssertFailureAndThrow(const char* s1, const char* file, int line)
{
    std::fprintf(stderr, "%s at %s:%d\n", s1, file, line);
    std::fflush(stderr);
    // printCurrentBacktrace();
    throw std::runtime_error(s1);
}

bool bigDivideUnsigned(uint64_t& result, uint64_t A, uint64_t B, uint64_t C,
                  Rounding rounding)
{
    releaseAssertOrThrow(C > 0);

    // update when moving to (signed) int128
    uint128_t a(A);
    uint128_t b(B);
    uint128_t c(C);
    uint128_t x = rounding == ROUND_DOWN ? (a * b) / c : (a * b + c - 1u) / c;

    result = (uint64_t)x;
    return (x <= UINT64_MAX);
}

// calculates A*B/C when A*B overflows 64bits
bool bigDivide(int64_t& result, int64_t A, int64_t B, int64_t C, Rounding rounding)
{
    bool res;
    releaseAssertOrThrow((A >= 0) && (B >= 0) && (C > 0));
    uint64_t r2;
    res =
        bigDivideUnsigned(r2, (uint64_t)A, (uint64_t)B, (uint64_t)C, rounding);
    if (res)
    {
        res = r2 <= INT64_MAX;
        result = r2;
    }
    return res;
}

int64_t bigDivideOrThrow(int64_t A, int64_t B, int64_t C, Rounding rounding)
{
    int64_t res;
    if (!bigDivide(res, A, B, C, rounding))
    {
        throw std::overflow_error("overflow while performing bigDivide");
    }
    return res;
}

bool bigDivideUnsigned128(uint64_t& result, uint128_t const& a, uint64_t B,
                     Rounding rounding)
{
    releaseAssertOrThrow(B != 0);

    // update when moving to (signed) int128
    uint128_t b(B);

    // We need to handle the case a + b - 1 > UINT128_MAX separately if rounding
    // up, since in this case a + b - 1 would overflow uint128_t. This is
    // equivalent to a > UINT128_MAX - (b - 1), where b >= 1 by assumption.
    // This is not a limitation of using uint128_t, since even if intermediate
    // values could not overflow we would still find that
    //     (a + b - 1) / b
    //         > UINT128_MAX / b
    //         >= UINT128_MAX / UINT64_MAX
    //         = ((UINT64_MAX + 1) * (UINT64_MAX + 1) - 1) / UINT64_MAX
    //         = (UINT64_MAX * UINT64_MAX + 2 * UINT64_MAX) / UINT64_MAX
    //         = UINT64_MAX + 2
    // which would have overflowed uint64_t anyway.
    uint128_t const UINT128_MAX = uint128_max();
    if ((rounding == ROUND_UP) && (a > UINT128_MAX - (b - 1u)))
    {
        return false;
    }

    uint128_t x = rounding == ROUND_DOWN ? a / b : (a + b - 1u) / b;

    result = (uint64_t)x;

    return (x <= UINT64_MAX);
}

bool bigDivide128(int64_t& result, uint128_t const& a, int64_t B, Rounding rounding)
{
    releaseAssertOrThrow(B > 0);

    uint64_t r2;
    bool res = bigDivideUnsigned128(r2, a, (uint64_t)B, rounding);
    if (res)
    {
        res = r2 <= INT64_MAX;
        result = r2;
    }
    return res;
}

int64_t bigDivideOrThrow128(uint128_t const& a, int64_t B, Rounding rounding)
{
    int64_t res;
    if (!bigDivide128(res, a, B, rounding))
    {
        throw std::overflow_error("overflow while performing bigDivide");
    }
    return res;
}

uint128_t bigMultiplyUnsigned(uint64_t a, uint64_t b)
{
    uint128_t A(a);
    uint128_t B(b);
    return A * B;
}

uint128_t bigMultiply(int64_t a, int64_t b)
{
    releaseAssertOrThrow((a >= 0) && (b >= 0));
    return bigMultiplyUnsigned((uint64_t)a, (uint64_t)b);
}

/* Excerpt from OfferExchange.cpp begins */

// Check that the relative error between the price and the effective price does
// not exceed 1%. If canFavorWheat == true then this function does an asymmetric
// check such that error favoring the seller of wheat can be unbounded, while
// the relative error between the price and the effective price does not exceed
// 1% if it is favoring the seller of sheep. The functionality of canFavorWheat
// is required for PathPayment.
bool
checkPriceErrorBound(Price price, int64_t wheatReceive, int64_t sheepSend,
                     bool canFavorWheat)
{
    // Let K = 100 / threshold, where threshold is the maximum relative error in
    // percent (so in this case, threshold = 1%). Then we can rearrange the
    // formula for relative error as follows:
    //     abs(price - effPrice) <= price / K
    //     price.d * abs(price - effPrice) <= price.n / K
    //     abs(price.n - price.d * effPrice) <= price.n / K
    //     abs(price.n * effPrice.d - price.d * effPrice.n)
    //         <= price.n * effPrice.d / K
    //     abs(K * price.n * effPrice.d - K * price.d * effPrice.n)
    //         <= price.n * effPrice.d

    // These never overflow since price.n and price.d are int32_t
    int64_t errN = (int64_t)100 * (int64_t)price.n;
    int64_t errD = (int64_t)100 * (int64_t)price.d;

    uint128_t lhs = bigMultiply(errN, wheatReceive);
    uint128_t rhs = bigMultiply(errD, sheepSend);
    
    if (canFavorWheat && rhs > lhs)
    {
        return true;
    }

    uint128_t absDiff = (lhs > rhs) ? (lhs - rhs) : (rhs - lhs);
    uint128_t cap = bigMultiply(price.n, wheatReceive);
    return (absDiff <= cap);
}

static uint128_t calculateOfferValue(int32_t priceN, int32_t priceD, int64_t maxSend, int64_t maxReceive)
{
    uint128_t sendValue = bigMultiply(maxSend, priceN);
    uint128_t receiveValue = bigMultiply(maxReceive, priceD);
    return std::min({sendValue, receiveValue});
}

// exchangeV10 is a system for crossing offers that provides guarantees
// regarding the direction and magnitude of rounding errors:
// - When considering two crossing offers subject to a variety of limits,
//   exchangeV10 has a consistent approach to determining which offer is larger.
//   The smaller offer is always removed from the book.
// - When two offers cross, the rounding error will favor the offer that remains
//   in the book.
// - The rounding error will not favor either party by more than 1% (except in
//   the case of path payment, where the rounding error can favor the offer in
//   the book by an arbitrary amount). If the rounding error would exceed 1%,
//   no trade occurs and the smaller offer is removed.
//
// As mentioned above, exchangeV10 retains the same guarantees when it is used
// to perform path payment except that the rounding error can favor the offer in
// the book by an arbitrary amount. This behavior is required in order to
// guarantee that, if the amount offered in the book exceeds maxWheatReceive
// after every offer is adjusted, then exchangeV10 will satisfy the constraint
// imposed by path payment of wheatReceive == maxWheatReceive. We note that this
// behavior is acceptable because path payment uses maxSend to determine whether
// the operation should succeed given the final effective price. If excessive
// rounding occurs, then path payment will fail because maxSend was exceeded.
//
// The quantities wheatValue and sheepValue play a central role in this function
// so it is worth discussing their significance. If we were working in arbitrary
// precision arithmetic, we would find that the value of the wheat offer in
// terms of sheep is
//     wheatOfferInTermsOfSheep = min(maxWheatSend * price, maxSheepReceive)
// and the size of the sheep offer in terms of sheep is
//     sheepOfferInTermsOfSheep = min(maxWheatReceive * price, maxSheepSend)
// Then we can see that the wheat offer is larger than the sheep offer if
//     wheatOfferInTermsOfSheep > sheepOfferInTermsOfSheep
// We are not, however, working in arbitrary precision arithmetic so we proceed
// by multiplying by price.d which yields
//     wheatOfferInTermsOfSheep * price.d
//         = min(maxWheatSend * price.n, maxSheepReceive * price.d)
//         = wheatValue
// and
//     sheepOfferInTermsOfSheep * price.d
//         = min(maxWheatReceive * price.n, maxSheepSend * price.d)
//         = sheepValue
// where both wheatValue and sheepValue are now integers. Clearly it is
// equivalent to say that the wheat offer is larger than the sheep offer if
//     wheatValue > sheepValue
// From this analysis, wheatValue can be thought of as the rescaled size of the
// wheat offer in terms of sheep after considering all limits. Analogously,
// sheepValue can be thought of as the rescaled size of the sheep offer in terms
// of sheep after considering all limits.
//
// If round == NORMAL --------------------------------------------------------
// The main objective here is to prove that rounding favours orders that remain
// in the book
// --------------------------------------------------------
// We first consider the case (wheatStays && price.n > price.d).
// Note: In this case, the wheat offer is greater and will likely have
//  a remainder that will stay in the book. With that context, we observe that
//  the price favours wheat since one gets more sheep per unit of wheat.
// Then
//     wheatReceive = floor(sheepValue / price.n)
//                  <= sheepValue / price.n
//                  <= (maxWheatReceive * price.n) / price.n
//                  = maxWheatReceive
//     wheatReceive = floor(sheepValue / price.n)
//                  <= sheepValue / price.n
//                  < wheatValue / price.n
//                  <= maxWheatSend
// so wheatReceive cannot exceed its limits. Similarly,
//     sheepSend = ceil(wheatReceive * price.n / price.d)
//               = ceil(floor(sheepValue / price.n) * price.n / price.d)
//               <= ceil((sheepValue / price.n) * price.n / price.d)
//               = ceil(sheepValue / price.d)
//               <= ceil((maxSheepSend * price.d) / price.d)
//               = maxSheepSend
//     sheepSend = ceil(wheatReceive * price.n / price.d)
//               = ceil(floor(sheepValue / price.n) * price.n / price.d)
//               <= ceil((sheepValue / price.n) * price.n / price.d)
//               = ceil(sheepValue / price.d)
//               <= ceil(wheatValue / price.d)
//               <= ceil((maxSheepReceive * price.d) / price.d)
//               = maxSheepReceive
// so sheepSend cannot exceed its limits. Because the limits for both
// wheatReceive and sheepSend are int64_t, neither bigDivide can fail. Now the
// effective price would be
//     sheepSend / wheatReceive
//         = ceil(wheatReceive * price.n / price.d) / wheatReceive
//         >= (wheatReceive * price.n / price.d) / wheatReceive
//         = price.n / price.d
// so in this case the seller of wheat is favored.
//
// We next consider the case (wheatStays && price.n <= price.d).
// Note: In this case, the wheat offer is still greater and will
//  likely have a remainder that will stay in the book. However, unlike before,
//  the price does not favour wheat since one or more wheat per unit of sheep.
// Then
//     sheepSend = floor(sheepValue / price.d)
//               <= sheepValue / price.d
//               <= maxSheepSend
//     sheepSend = floor(sheepValue / price.d)
//               <= sheepValue / price.d
//               < wheatValue / price.d
//               <= maxSheepReceive
// so sheepSend cannot exceed its limits. Similarly,
//     wheatReceive = floor(sheepSend * price.d / price.n)
//                  <= sheepSend * price.d / price.n
//                  <= floor(sheepValue / price.d) * price.d / price.n
//                  <= (sheepValue / price.d) * price.d / price.n
//                  = sheepValue / price.n
//                  <= maxWheatReceive
//     wheatReceive = floor(sheepSend * price.d / price.n)
//                  <= sheepSend * price.d / price.n
//                  <= floor(sheepValue / price.d) * price.d / price.n
//                  <= (sheepValue / price.d) * price.d / price.n
//                  = sheepValue / price.n
//                  < wheatValue / price.n
//                  <= maxWheatSend
// so wheatReceive cannot exceed its limits. Because the limits for both
// wheatReceive and sheepSend are int64_t, neither bigDivide can fail. Now the
// effective price would be
//     sheepSend / wheatReceive
//         = sheepSend / floor(sheepSend * price.d / price.n)
//         >= sheepSend / (sheepSend * price.d / price.n)
//         = price.n / price.d
// so in this case the seller of wheat is favored.
//
// We now shift attention to the case (!wheatStays && price.n > price.d).
// Note: In this case, the wheat offer is lesser. This means there will likely
//  be a sheep offer that will stay in the book. With that context, we observe
//  the price favours wheat since one gets more sheep per unit of wheat.
// Then
//     wheatReceive = floor(wheatValue / price.n)
//                  <= wheatValue / price.n
//                  <= maxWheatSend
//     wheatReceive = floor(wheatValue / price.n)
//                  <= wheatValue / price.n
//                  <= sheepValue / price.n
//                  = maxWheatSend
// so wheatReceive cannot exceed its limits. Similarly,
//     sheepSend = floor(wheatReceive * price.n / price.d)
//               <= wheatReceive * price.n / price.d
//               = floor(wheatValue / price.n) * price.n / price.d
//               <= (wheatValue / price.n) * price.n / price.d
//               = wheatValue / price.d
//               <= maxSheepReceive
//     sheepSend = floor(wheatReceive * price.n / price.d)
//               <= wheatReceive * price.n / price.d
//               = floor(wheatValue / price.n) * price.n / price.d
//               <= (wheatValue / price.n) * price.n / price.d
//               = wheatValue / price.d
//               < sheepValue / price.d
//               <= maxSheepSend
// so sheepSend cannot exceed its limits. Because the limits for both
// wheatReceive and sheepSend are int64_t, neither bigDivide can fail. Now the
// effective price would be
//     sheepSend / wheatReceive
//         = floor(wheatReceive * price.n / price.d) / wheatReceive
//         <= (wheatReceive * price.n / price.d) / wheatReceive
//         = price.n / price.d
// so in this case the seller of sheep is favored.
//
// Finally, we come to the case (!wheatStays && price.n <= price.d)
// Note: In this case, the wheat offer is lesser. This means there will likely
//  be a sheep offer that will stay in the book. With that context, we observe
//  the price favours sheep since one gets equal or more wheat per unit of sheep
// Then
//     sheepSend = floor(wheatValue / price.d)
//               <= wheatValue / price.d
//               <= maxSheepReceive
//     sheepSend = floor(wheatValue / price.d)
//               <= wheatValue / price.d
//               <= sheepValue / price.d
//               = maxSheepSend
// so sheepSend cannot exceed its limits. Similarly,
//     wheatReceive = ceil(sheepSend * price.d / price.n)
//                  = ceil(floor(wheatValue / price.d) * price.d / price.n)
//                  <= ceil((wheatValue / price.d) * price.d / price.n)
//                  = ceil(wheatValue / price.n)
//                  <= maxWheatSend
//     wheatReceive = ceil(sheepSend * price.d / price.n)
//                  = ceil(floor(wheatValue / price.d) * price.d / price.n)
//                  <= ceil((wheatValue / price.d) * price.d / price.n)
//                  = ceil(wheatValue / price.n)
//                  <= ceil(sheepValue / price.n)
//                  <= maxWheatReceive
// so wheatReceive cannot exceed its limits. Because the limits for both
// wheatReceive and sheepSend are int64_t, neither bigDivide can fail. Now the
// effective price would be
//     sheepSend / wheatReceive
//         = sheepSend / ceil(sheepSend * price.d / price.n)
//         <= sheepSend / (sheepSend * price.d / price.n)
//         = price.n / price.d
// so in this case the seller of sheep is favored.
//
// If round == PATH_PAYMENT_STRICT_RECEIVE ------------------------------------
// We first consider the case wheatStays. In this case, we guarantee that the
// effective price favors wheat
//     sheepSend / wheatReceive >= price.n / price.d
// Path payment can only succeed if, after the last offer is crossed,
// wheatReceive == maxWheatReceive. Because wheatStays, we know that this is the
// last offer that will be crossed. Then if we are to satisfy both constraints,
// it is necessary that
//     sheepSend / maxWheatReceive >= price.n / price.d
// which is equivalent to
//     sheepSend * price.d >= maxWheatReceive * price.n
// But sheepSend <= maxSheepSend, so this can only be satisfied if
//     maxSheepSend * price.d >= maxWheatReceive * price.n             (*)
// If this constraint is not satisfied, then the operation must fail.
//
// We will now show that the case (wheatStays && price.n > price.d) along with
// the constraint (*) guarantees that wheatReceive == maxWheatReceive. In this
// case we have
//     sheepValue = maxWheatReceive * price.n
// so
//     wheatReceive = floor(sheepValue / price.n)
//                  = maxWheatReceive
// and
//     sheepSend = ceil(wheatReceive * price.n / price.d)
//                = ceil(maxWheatReceive * price.n / price.d)
// Clearly the operation succeeds.
//
// We will next show that if wheatReceive == maxWheatReceive in the case
// (wheatStays && price.n <= price.d) then
//     sheepSend = ceil(maxWheatReceive * price.n / price.d)
// so the outcome is unchanged from (wheatStays && price.n > price.d). Note that
// the constraint (*) is satisfied since it is a necessary condition to have
// wheatReceive == maxWheatReceive. Then
//     sheepValue = maxWheatReceive * price.n
// so
//     sheepSend = floor(sheepValue / price.d)
//               = floor(maxWheatReceive * price.n / price.d)
// which is equivalent to
//     maxWheatReceive * price.n / price.d - 1 < sheepSend
//         <= maxWheatReceive * price.n / price.d
// Furthermore, we have
//     maxWheatReceive = floor(sheepSend * price.d / price.n)
// which is equivalent to
//     maxWheatReceive <= sheepSend * price.d / price.n < maxWheatReceive + 1
//     maxWheatReceive * price.n / price.d <= sheepSend
//         < (maxWheatReceive + 1) * price.n / price.d
// Combining the above inequalities, we find
//     maxWheatReceive * price.n / price.d <= sheepSend
//         <= maxWheatReceive * price.n / price.d
// so clearly we have
//     sheepSend = maxWheatReceive * price.n / price.d
// But sheepSend is an integer, so
//     sheepSend = ceil(sheepSend)
//               = ceil(maxWheatReceive * price.n / price.d)
// which completes this argument.
//
// Now we come to the reason that when wheatStays we handle all cases the same
// regardless of the price. The previous argument showed that the two cases are
// identical when wheatReceive == maxWheatReceive. But it is possible in the
// case (wheatStays && price.n <= price.d) that wheatReceive < maxWheatReceive.
// Consider the case
//     price = 2/3
//     maxWheatSend = 150
//     maxWheatReceive = 101
//     maxSheepSend = INT64_MAX
//     maxSheepReceive = INT64_MAX
//     round = PATH_PAYMENT_STRICT_RECEIVE
// so
//     wheatValue = min(2 * 150, 3 * INT64_MAX) = 300
//     sheepValue = min(3 * INT64_MAX, 2 * 101) = 202
// which implies (wheatStays && price.n <= price.d). Then
//     sheepSend = floor(sheepValue / price.d)
//               = floor(202 / 3) = 67
//     wheatReceive = floor(sheepSend * price.d / price.n)
//                  = floor(67 * 3 / 2) = 100
// and clearly wheatReceive == 100 != 101 == maxWheatReceive.
//
// At this point we have determined what must occur if wheatStays but have not
// addressed the case !wheatStays. If wheatReceive = maxWheatReceive, then the
// operation succeeds. Otherwise, if sheepSend = maxSheepSend then the operation
// fails. If wheatReceive < maxWheatReceive and sheepSend < maxSheepSend, then
// the operation will cross additional offers since !wheatStays.
//
// If round == PATH_PAYMENT_STRICT_SEND ---------------------------------------
// We first consider the case (wheatStays && price.n > price.d). Then
//     wheatReceive = floor(sheepValue / price.n)
//                  <= floor((maxWheatReceive * price.n) / price.n)
//                  = maxWheatReceive
//     wheatReceive = floor(sheepValue / price.n)
//                  <= floor(wheatValue / price.n)
//                  <= floor((maxWheatSend * price.n) / price.n)
//                  = maxWheatSend
// so wheatReceive cannot exceed its limits. Because wheatStays, we know that
// this is the last offer that will be crossed so
//     sheepSend = min(maxSheepSend, maxSheepReceive)
// Therefore if maxSheepSend > maxSheepReceive then the operation must fail. Now
// the effective price would be
//     sheepSend / wheatReceive
//         = min(maxSheepSend / wheatReceive, maxSheepReceive / wheatReceive)
// where
//     maxSheepSend / wheatReceive
//         = maxSheepSend / floor(sheepValue / price.n)
//         >= maxSheepSend / (sheepValue / price.n)
//         = maxSheepSend * price.n / sheepValue
//         >= maxSheepSend * price.n / (maxSheepSend * price.d)
//         = price.n / price.d
//     maxSheepReceive / wheatReceive
//         = maxSheepReceive / floor(sheepValue / price.n)
//         >= maxSheepReceive / (sheepValue / price.n)
//         = maxSheepReceive * price.n / sheepValue
//         > maxSheepReceive * price.n / wheatValue
//         >= maxSheepReceive * price.n / (maxSheepReceive * price.d)
//         = price.n / price.d
// so in this case the seller of wheat is favored.
//
// Suppose that wheatReceive < maxWheatReceive. This implies that
//     sheepValue = maxSheepSend * price.d
// for otherwise we would have
//     sheepValue = maxWheatReceive * price.n
// and
//     wheatReceive = floor(sheepValue / price.n)
//                  = floor((maxWheatReceive * price.n) / price.n)
//                  = maxWheatReceive
// It follows from this that
//     maxSheepReceive > maxSheepSend
// for otherwise
//     maxSheepReceive <= maxSheepSend
// and
//     wheatValue <= maxSheepReceive * price.d
//                <= maxSheepSend * price.d
//                = sheepValue
// which contradicts the assumption that wheatStays. We claim that wheatReceive
// is the maximum amount of wheat that could be received without favoring the
// seller of sheep. To see this, observe that
//     sheepSend / (wheatReceive + 1)
//         = maxSheepSend / (floor(sheepValue / price.n) + 1)
//         = maxSheepSend / (floor(maxSheepSend * price.d / price.n) + 1)
//         <= maxSheepSend / ceil(maxSheepSend * price.d / price.n)
//         <= maxSheepSend / (maxSheepSend * price.d / price.n)
//         <= price.n / price.d
// Now for any integer K >= 1 we must have
//     sheepSend / (wheatReceive + K)
//         <= sheepSend / (wheatReceive + 1)
//         <= price.n / price.d
// which proves the claim.
//
// At this point we have determined what must occur if wheatStays but have not
// addressed the case !wheatStays. If sheepSend = maxSheepSend, then the
// operation succeeds. Otherwise, if wheatReceive = maxWheatReceive then the
// operation fails. If sheepSend < maxSheepSend and wheatReceive <
// maxWheatReceive, then the operation will cross additional offers since
// !wheatStays.
ExchangeResultV10 exchangeV10(Price price, int64_t maxWheatSend, int64_t maxWheatReceive,
            int64_t maxSheepSend, int64_t maxSheepReceive, RoundingType round)
{
    // ZoneScoped;
    auto beforeThresholds = exchangeV10WithoutPriceErrorThresholds(
        price, maxWheatSend, maxWheatReceive, maxSheepSend, maxSheepReceive,
        round);
    return applyPriceErrorThresholds(price, beforeThresholds.numWheatReceived,
                                     beforeThresholds.numSheepSend,
                                     beforeThresholds.wheatStays, round);
}

// See comment before exchangeV10 for proof of some important properties. We
// will prove that for rounding modes NORMAL and PATH_PAYMENT_STRICT_RECEIVE,
// wheatReceive == 0 if and only if sheepSend == 0. We will also prove that for
// rounding mode PATH_PAYMENT_STRICT_SEND, sheepSend > 0 is guaranteed.
//
// We first address the case (!wheatStays && price.n > price.d). Then it follows
// from wheatReceive = 0 that
//     sheepSend = floor(wheatReceive * price.n / price.d)
// so sheepSend = 0. Similarly, if sheepSend = 0 then
//     sheepSend = floor(wheatReceive * price.n / price.d)
//               >= floor(wheatReceive)
//               = wheatReceive
// so 0 <= wheatReceive <= 0 implies wheatReceive = 0.
//
// Similarly, we investigate when (!wheatStays && price.n <= price.d). Then it
// follows from sheepSend = 0 that
//     wheatReceive = ceil(sheepSend * price.d / price.n)
// so wheatReceive = 0. Similarly, if wheatReceive = 0 then
//     wheatReceive = ceil(sheepSend * price.d / price.n)
//                  >= ceil(sheepSend)
//                  = sheepSend
// so 0 <= sheepSend <= 0 implies sheepSend = 0.
//
// These first two results hold regardless of the rounding mode.
//
// If round == NORMAL ---------------------------------------------------------
// We turn our attention to the case (wheatStays && price.n > price.d). Then it
// follows from wheatReceive = 0 that
//     sheepSend = ceil(wheatReceive * price.n / price.d)
// so sheepSend = 0. Similarly, if sheepSend = 0 then
//     sheepSend = ceil(wheatReceive * price.n / price.d)
//               >= ceil(wheatReceive)
//               = wheatReceive
// so 0 <= wheatReceive <= 0 implies wheatReceive = 0.
//
// Next consider the case (wheatStays && price.n <= price.d). Then it follows
// from sheepSend = 0 that
//     wheatReceive = floor(sheepSend * price.d / price.n)
// so wheatReceive = 0. Similarly, if wheatReceive = 0 then
//     wheatReceive = floor(sheepSend * price.d / price.n)
//                  >= floor(sheepSend)
//                  = sheepSend
// so 0 <= sheepSend <= 0 implies sheepSend = 0.
//
// If round == PATH_PAYMENT_STRICT_RECEIVE ------------------------------------
// The case (wheatStays && price.n > price.d) is identical to the analogous case
// when round == NORMAL.
//
// We now consider the case (wheatStays && price.n <= price.d). Then it follows
// from wheatReceive = 0 that
//     sheepSend = ceil(wheatReceive * price.n / price.d)
// so sheepSend = 0. Similarly, if sheepSend = 0 then
//     sheepSend = ceil(wheatReceive * price.n / price.d)
//               >= ceil(wheatReceive * 1 / INT32_MAX)
//               = ceil(wheatReceive / INT32_MAX)
// Suppose that wheatReceive > 0. Then
//     sheepSend >= ceil(1 / INT32_MAX) = 1
// which is a contradiction, so 0 <= wheatReceive <= 0 implies wheatReceive = 0.
//
// If round == PATH_PAYMENT_STRICT_SEND ---------------------------------------
// Finally, we address the more complicated case (wheatStays). Suppose that we
// have sheepSend = 0. Then clearly maxSheepSend = 0 or maxSheepReceive = 0. But
// if maxSheepSend = 0, then we should have already stopped crossing offers
// because no more can be sent. Similarly if maxSheepReceive = 0, then the offer
// should have already been removed from the order book because no more can be
// received. In either case, we have reached a contradiction because we would
// not be crossing in either case. We conclude that sheepSend > 0.
ExchangeResultV10 exchangeV10WithoutPriceErrorThresholds(Price price, int64_t maxWheatSend,
                                       int64_t maxWheatReceive,
                                       int64_t maxSheepSend,
                                       int64_t maxSheepReceive,
                                       RoundingType round)
{
    uint128_t wheatValue =
        calculateOfferValue(price.n, price.d, maxWheatSend, maxSheepReceive);
    uint128_t sheepValue =
        calculateOfferValue(price.d, price.n, maxSheepSend, maxWheatReceive);
    bool wheatStays = (wheatValue > sheepValue);

    int64_t wheatReceive;
    int64_t sheepSend;
    if (wheatStays)
    {
        if (round == RoundingType::PATH_PAYMENT_STRICT_SEND)
        {
            wheatReceive = bigDivideOrThrow128(sheepValue, price.n, ROUND_DOWN);
            sheepSend = std::min({maxSheepSend, maxSheepReceive});
        }
        else if (price.n > price.d || // Wheat is more valuable
                 round == RoundingType::PATH_PAYMENT_STRICT_RECEIVE)
        {
            wheatReceive = bigDivideOrThrow128(sheepValue, price.n, ROUND_DOWN);
            sheepSend =
                bigDivideOrThrow(wheatReceive, price.n, price.d, ROUND_UP);
        }
        else // Sheep is more valuable
        {
            sheepSend = bigDivideOrThrow128(sheepValue, price.d, ROUND_DOWN);
            wheatReceive =
                bigDivideOrThrow(sheepSend, price.d, price.n, ROUND_DOWN);
        }
    }
    else
    {
        if (price.n > price.d) // Wheat is more valuable
        {
            wheatReceive = bigDivideOrThrow128(wheatValue, price.n, ROUND_DOWN);
            sheepSend =
                bigDivideOrThrow(wheatReceive, price.n, price.d, ROUND_DOWN);
        }
        else // Sheep is more valuable
        {
            sheepSend = bigDivideOrThrow128(wheatValue, price.d, ROUND_DOWN);
            wheatReceive =
                bigDivideOrThrow(sheepSend, price.d, price.n, ROUND_UP);
        }
    }

    // Neither of these should ever throw.
    if (wheatReceive < 0 ||
        wheatReceive > std::min({maxWheatReceive, maxWheatSend}))
    {
        throw std::runtime_error("wheatReceive out of bounds");
    }
    if (sheepSend < 0 || sheepSend > std::min({maxSheepReceive, maxSheepSend}))
    {
        throw std::runtime_error("sheepSend out of bounds");
    }

    ExchangeResultV10 res;
    res.numWheatReceived = wheatReceive;
    res.numSheepSend = sheepSend;
    res.wheatStays = wheatStays;
    return res;
}

// See comment before exchangeV10.
ExchangeResultV10 applyPriceErrorThresholds(Price price, int64_t wheatReceive, int64_t sheepSend,
                          bool wheatStays, RoundingType round)
{
    if (wheatReceive > 0 && sheepSend > 0)
    {
        uint128_t wheatReceiveValue = bigMultiply(wheatReceive, price.n);
        uint128_t sheepSendValue = bigMultiply(sheepSend, price.d);

        // ExchangeV10 guarantees that if wheat stays then the wheat seller
        // must be favored. Similarly, if sheep stays then the sheep seller
        // must be favored.
        if (wheatStays && sheepSendValue < wheatReceiveValue)
        {
            throw std::runtime_error("favored sheep when wheat stays");
        }
        if (!wheatStays && sheepSendValue > wheatReceiveValue)
        {
            throw std::runtime_error("favored wheat when sheep stays");
        }

        if (round == RoundingType::NORMAL)
        {
            // Both sellers must get a price no more than 1% worse than the
            // price crossed. Otherwise, no trade occurs.
            if (!checkPriceErrorBound(price, wheatReceive, sheepSend, false))
            {
                sheepSend = 0;
                wheatReceive = 0;
            }
        }
        else
        {
            // When the wheat seller is favored, they can be arbitrarily favored
            // since path payment has a sendMax or destMin parameter to
            // determine whether a price was acceptable. When the sheep seller
            // is favored, we still want the wheat seller to get a price no more
            // than 1% worse than the price crossed. The sheep seller can only
            // be favored if !wheatStays, and in this case the entire offer will
            // be taken. But the offer was adjusted immediately before
            // exchangeV10, so we know that it satisfies the threshold in this
            // case.
            if (!checkPriceErrorBound(price, wheatReceive, sheepSend, true))
            {
                throw std::runtime_error("exceeded price error bound");
            }
        }
    }
    else
    {
        switch (round)
        {
        case RoundingType::PATH_PAYMENT_STRICT_SEND:
            // For PathPaymentStrictSend, there are situations when the sender
            // must sell sheep for no wheat in order to send exactly the
            // specified amount. This is acceptable because there is still the
            // overall constraint on amount received. However, it should never
            // happen that the sender sells no sheep and we throw in this case.
            if (sheepSend == 0)
            {
                throw std::runtime_error("invalid amount of sheep sent");
            }
            break;
        default:
            // Based on the proof proceeding
            // exchangeV10WithoutPriceErrorThresholds, we should already have
            // wheatReceive = 0 and sheepSend = 0. We set it explicitly for
            // clarity.
            wheatReceive = 0;
            sheepSend = 0;
            break;
        }
    }

    ExchangeResultV10 res;
    res.numWheatReceived = wheatReceive;
    res.numSheepSend = sheepSend;
    res.wheatStays = wheatStays;
    return res;
}

} // namespace stellar