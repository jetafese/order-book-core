# order-book-core

https://github.com/stellar/stellar-core/blob/master/src/transactions/OfferExchange.cpp#L227-L699

# Current Proof Status
```
 Proof summary for theory ExchangeV10
    big_div_TCC1..........................proved - complete   [SHOSTAK](0.00 s)
    big_div_TCC2..........................unfinished          [SHOSTAK](0.00 s)
    big_div_TCC3..........................proved - complete   [SHOSTAK](0.00 s)
    big_div_TCC4..........................unfinished          [SHOSTAK](0.00 s)
    big_div_numden_TCC1...................unfinished          [SHOSTAK](0.00 s)
    big_div_numden_TCC2...................unfinished          [SHOSTAK](0.00 s)
    calculateOfferValue_TCC1..............unfinished          [SHOSTAK](0.00 s)
    calculateOfferValue_TCC2..............unfinished          [SHOSTAK](0.00 s)
    calculateOfferValue_TCC3..............unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC1...proved - complete   [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC2...proved - complete   [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC3...proved - incomplete [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC4...proved - incomplete [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC5...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC6...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC7...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC8...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC9...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC10...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC11...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC12...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC13...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC14...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC15...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC16...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC17...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC18...unfinished          [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC19...unfinished          [SHOSTAK](0.00 s)
    Theory ExchangeV10 totals: 28 formulas, 28 attempted, 6 succeeded (0.01 s)

 Proof summary for theory ApplyPriceErrorThresholds_Lemmas
    thresholds_normal_bounded_error_no_trade_TCC1...proved - incomplete [SHOSTAK](0.00 s)
    thresholds_normal_bounded_error_no_trade_TCC2...unfinished          [SHOSTAK](0.00 s)
    thresholds_normal_bounded_error_no_trade_TCC3...proved - incomplete [SHOSTAK](0.00 s)
    thresholds_normal_bounded_error_no_trade...proved - incomplete [SHOSTAK](0.00 s)
    thresholds_normal_bounded_error_successful...proved - incomplete [SHOSTAK](0.00 s)
    thresholds_path_error_bounded_favor_sheep_TCC1...proved - incomplete [SHOSTAK](0.00 s)
    thresholds_path_error_bounded_favor_sheep...proved - incomplete [SHOSTAK](0.00 s)
    thresholds_path_error_unbounded_favor_wheat...proved - incomplete [SHOSTAK](0.00 s)
    thresholds_normal_no_trade_if_wR_or_sS_equal_zero...proved - incomplete [SHOSTAK](0.00 s)
    thresholds_path_receive_no_trade_if_wR_or_sS_equal_zero...proved - incomplete [SHOSTAK](0.00 s)
    thresholds_path_send_sS_not_zero_if_wR_or_sS_equal_zero...proved - incomplete [SHOSTAK](0.00 s)
    record_path_send_sS_not_zero_if_wR_or_sS_equal_zero...proved - incomplete [SHOSTAK](0.00 s)
    Theory ApplyPriceErrorThresholds_Lemmas totals: 12 formulas, 12 attempted, 11 succeeded (0.00 s)

 Proof summary for theory ApplyPriceErrorThresholds
    Theory ApplyPriceErrorThresholds totals: 0 formulas, 0 attempted, 0 succeeded (0.00 s)

 Proof summary for theory CheckPriceErrorBound_Lemmas
    uint128_gte_zero......................proved - complete   [SHOSTAK](0.00 s)
    lemma_within_one_percent_normal_TCC1...proved - complete   [SHOSTAK](0.00 s)
    lemma_within_one_percent_normal_TCC2...proved - complete   [SHOSTAK](0.00 s)
    lemma_within_one_percent_normal_TCC3...proved - complete   [SHOSTAK](0.00 s)
    lemma_within_one_percent_normal.......proved - incomplete [SHOSTAK](0.00 s)
    lemma_within_one_percent_path_favor_sheep...proved - incomplete [SHOSTAK](0.00 s)
    lemma_arbitrary_error.................proved - incomplete [SHOSTAK](0.00 s)
    Theory CheckPriceErrorBound_Lemmas totals: 7 formulas, 7 attempted, 7 succeeded (0.00 s)

 Proof summary for theory CheckPriceErrorBound
    big_mul_TCC1..........................unfinished          [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC1.............proved - complete   [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC2.............proved - complete   [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC3.............proved - complete   [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC4.............proved - complete   [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC5.............proved - incomplete [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC6.............proved - incomplete [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC7.............proved - incomplete [SHOSTAK](0.00 s)
    Theory CheckPriceErrorBound totals: 8 formulas, 8 attempted, 7 succeeded (0.00 s)

 Proof summary for theory top
    Theory top totals: 0 formulas, 0 attempted, 0 succeeded (0.00 s)

Grand Totals: 55 proofs, 55 attempted, 31 succeeded (0.02 s)
```