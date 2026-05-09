#include <cstdint>
#include <exception>
#include <iostream>

#include "OfferExchange.h"

int
main()
{
    stellar::Price const price{3, 2};
    int64_t const maxWheatSend = 0;
    int64_t const maxWheatReceive = 1;
    int64_t const maxSheepSend = 1;
    int64_t const maxSheepReceive = 1;
    stellar::RoundingType const round =
        stellar::RoundingType::PATH_PAYMENT_STRICT_SEND;

    std::cout << "exchangeV10 path-send TCC counterexample input\n";
    std::cout << "  price=(3/2), maxWheatSend=0, maxWheatReceive=1, "
                 "maxSheepSend=1, maxSheepReceive=1, "
                 "round=PATH_PAYMENT_STRICT_SEND\n";

    try
    {
        auto const res =
            stellar::exchangeV10(price, maxWheatSend, maxWheatReceive,
                                 maxSheepSend, maxSheepReceive, round);
        std::cout << "  unexpected result: wheatStays=" << res.wheatStays
                  << ", numWheatReceived=" << res.numWheatReceived
                  << ", numSheepSend=" << res.numSheepSend << "\n";
        return 1;
    }
    catch (std::exception const& e)
    {
        std::cout << "  reproduced exception: " << e.what() << "\n";
        return 0;
    }
}

