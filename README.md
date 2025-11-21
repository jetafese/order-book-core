# order-book-core

https://github.com/stellar/stellar-core/blob/master/src/transactions/OfferExchange.cpp#L227-L699

# Current Proof Status
```
 Proof summary for theory ExchangeV10_Lemmas
    wheatStays_exchangeV10WithoutPriceErrorThresholds...proved - incomplete [SHOSTAK](0.00 s)
    wheatStays_pathSend_nn................proved - incomplete [SHOSTAK](0.00 s)
    wheatStays_nztrade_moreSheep_than_wheat...proved - incomplete [SHOSTAK](0.01 s)
    not_wheatStays_nztrade_lessSheep_than_wheat...proved - incomplete [SHOSTAK](0.00 s)
    Theory ExchangeV10_Lemmas totals: 4 formulas, 4 attempted, 4 succeeded (0.02 s)

 Proof summary for theory ExchangeV10
    big_div_TCC1..........................proved - complete   [SHOSTAK](0.00 s)
    big_div_TCC2..........................proved - incomplete [SHOSTAK](0.00 s)
    big_div_TCC3..........................proved - complete   [SHOSTAK](0.00 s)
    big_div_TCC4..........................proved - incomplete [SHOSTAK](0.00 s)
    big_div_numden_TCC1...................proved - complete   [SHOSTAK](0.00 s)
    big_div_numden_TCC2...................proved - incomplete [SHOSTAK](0.00 s)
    big_div_numden_TCC3...................proved - complete   [SHOSTAK](0.00 s)
    big_div_numden_TCC4...................proved - incomplete [SHOSTAK](0.00 s)
    calculateOfferValue_TCC1..............proved - incomplete [SHOSTAK](0.00 s)
    exchangeV10WithoutPriceErrorThresholds_TCC1...unfinished          [SHOSTAK](0.00 s)
    Theory ExchangeV10 totals: 10 formulas, 10 attempted, 9 succeeded (0.01 s)

 Proof summary for theory ApplyPriceErrorThresholds_Lemmas
    thresholds_normal_bounded_error_no_trade_TCC1...proved - complete   [SHOSTAK](0.00 s)
    thresholds_normal_bounded_error_no_trade_TCC2...proved - complete   [SHOSTAK](0.00 s)
    thresholds_normal_bounded_error_no_trade...proved - complete   [SHOSTAK](0.00 s)
    thresholds_normal_bounded_error_successful...proved - complete   [SHOSTAK](0.00 s)
    thresholds_path_error_bounded_favor_sheep_TCC1...proved - complete   [SHOSTAK](0.00 s)
    thresholds_path_error_bounded_favor_sheep...proved - complete   [SHOSTAK](0.00 s)
    thresholds_path_error_unbounded_favor_wheat...proved - complete   [SHOSTAK](0.00 s)
    thresholds_normal_no_trade_if_wR_or_sS_equal_zero...proved - complete   [SHOSTAK](0.00 s)
    thresholds_path_receive_no_trade_if_wR_or_sS_equal_zero...proved - complete   [SHOSTAK](0.00 s)
    thresholds_path_send_sS_not_zero_if_wR_or_sS_equal_zero...proved - complete   [SHOSTAK](0.00 s)
    record_path_send_sS_not_zero_if_wR_or_sS_equal_zero...proved - complete   [SHOSTAK](0.00 s)
    Theory ApplyPriceErrorThresholds_Lemmas totals: 11 formulas, 11 attempted, 11 succeeded (0.00 s)

 Proof summary for theory ApplyPriceErrorThresholds
    Theory ApplyPriceErrorThresholds totals: 0 formulas, 0 attempted, 0 succeeded (0.00 s)

 Proof summary for theory CheckPriceErrorBound_Lemmas
    uint128_gte_zero......................proved - complete   [SHOSTAK](0.00 s)
    lemma_within_one_percent_normal_TCC1...proved - complete   [SHOSTAK](0.00 s)
    lemma_within_one_percent_normal_TCC2...proved - complete   [SHOSTAK](0.00 s)
    lemma_within_one_percent_normal.......proved - complete   [SHOSTAK](0.00 s)
    lemma_within_one_percent_path_favor_sheep_TCC1...proved - complete   [SHOSTAK](0.00 s)
    lemma_within_one_percent_path_favor_sheep_TCC2...proved - complete   [SHOSTAK](0.00 s)
    lemma_within_one_percent_path_favor_sheep...proved - complete   [SHOSTAK](0.00 s)
    lemma_arbitrary_error.................proved - complete   [SHOSTAK](0.00 s)
    Theory CheckPriceErrorBound_Lemmas totals: 8 formulas, 8 attempted, 8 succeeded (0.00 s)

 Proof summary for theory CheckPriceErrorBound
    big_mul_TCC1..........................proved - complete   [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC1.............proved - complete   [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC2.............proved - complete   [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC3.............proved - complete   [SHOSTAK](0.00 s)
    checkPriceErrorBound_TCC4.............proved - complete   [SHOSTAK](0.00 s)
    Theory CheckPriceErrorBound totals: 5 formulas, 5 attempted, 5 succeeded (0.00 s)

 Proof summary for theory top
    Theory top totals: 0 formulas, 0 attempted, 0 succeeded (0.00 s)

Grand Totals: 38 proofs, 38 attempted, 37 succeeded (0.03 s)
```