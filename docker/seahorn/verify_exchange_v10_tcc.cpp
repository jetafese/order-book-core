#include <cstdint>

extern "C" void __VERIFIER_error(void);

enum RoundingType
{
    NORMAL,
    PATH_PAYMENT_STRICT_RECEIVE,
    PATH_PAYMENT_STRICT_SEND
};

struct Price
{
    std::int64_t n;
    std::int64_t d;
};

struct State
{
    Price price;
    bool wheatStays;
    std::int64_t sS;
    std::int64_t wR;
    RoundingType round;
};

static bool
wf_state(State const& s)
{
    bool const pathSendZeroWheat =
        (s.wR == 0 && s.round == PATH_PAYMENT_STRICT_SEND) ? (s.sS != 0) : true;
    bool const wheatStaysNonzeroTrade =
        (s.wR > 0 && s.sS > 0 && s.wheatStays)
            ? (s.price.d * s.sS >= s.price.n * s.wR)
            : true;
    bool const sheepStaysNonzeroTrade =
        (s.wR > 0 && s.sS > 0 && !s.wheatStays)
            ? (s.price.n * s.wR >= s.price.d * s.sS)
            : true;

    return pathSendZeroWheat && wheatStaysNonzeroTrade &&
           sheepStaysNonzeroTrade;
}

int
main()
{
    // Concrete image of ExchangeV10.exchangeV10WithoutPriceErrorThresholds_TCC1:
    // price=(3/2), maxWheatSend=0, maxWheatReceive=1,
    // maxSheepSend=1, maxSheepReceive=1, round=PATH_PAYMENT_STRICT_SEND
    // yields wheatStays=false, wR=0, sS=0. That violates wf_state?'s first
    // conjunct: wR=0 and PATH_PAYMENT_STRICT_SEND implies sS != 0.
    State const state{{3, 2}, false, 0, 0, PATH_PAYMENT_STRICT_SEND};

    if (!wf_state(state))
    {
        __VERIFIER_error();
    }
    return 0;
}
