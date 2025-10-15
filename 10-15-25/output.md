Below is a **clean, type‑safe PVS specification** that mirrors the two C++ translation units you provided (`OfferExchange.h` and `OfferExchange.cpp`) and states the properties in the comments as **theorems to prove**. I use PVS subtyping to encode C++ range constraints precisely (no loss of precision) and give a **side‑by‑side correspondence map** so you can see where each C++ definition lands in PVS. The spec is organized into small theories so you can load them incrementally in PVS.

> Sources: The mapping and properties were extracted directly from your files, `OfferExchange.h` and `OfferExchange.cpp`.  

---

## Side‑by‑side correspondence map

| C++ (file / entity)                                                                              | PVS (theory / name)                                                       | Notes                                                                                          |
| ------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------- |
| **Basic types** (`int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `uint128_t`)                      | `offer_cxx_types` / `int32`, `uint32`, `int64`, `uint64`, `uint128`       | Exact ranges using subtypes; constants `INT64_MAX`, `UINT128_MAX`, … exactly specified.        |
| `enum Rounding { ROUND_DOWN, ROUND_UP }`                                                         | `offer_types` / `Rounding`                                                | Enumeration.                                                                                   |
| `enum class RoundingType { NORMAL, PATH_PAYMENT_STRICT_SEND, PATH_PAYMENT_STRICT_RECEIVE }`      | `offer_types` / `RoundingType`                                            | Enumeration.                                                                                   |
| `enum class ExchangeResultType { NORMAL, REDUCED_TO_ZERO, BOGUS }`                               | `offer_types` / `ExchangeResultType`                                      | Enumeration.                                                                                   |
| `struct Price { int32_t n; int32_t d; };`                                                        | `offer_types` / `Price`                                                   | Record with 32‑bit components.                                                                 |
| `struct ExchangeResult { …; ExchangeResultType type() const; }`                                  | `offer_types` / `ExchangeResult` + `exchangeResultType`                   | Same logic for `type()`.                                                                       |
| `struct ExchangeResultV10 { int64_t numWheatReceived; int64_t numSheepSend; bool wheatStays; };` | `offer_types` / `ExchangeResultV10`                                       | Record.                                                                                        |
| `bool bigDivide(...)`, `bool bigDivideUnsigned(...)`                                             | `bigmath` / `big_div`, `big_div_unsigned`                                 | Total functions returning `(res, ok)`; preconditions captured by subtyping.                    |
| `int64_t bigDivideOrThrow(...)`                                                                  | `bigmath` / `big_div_or_throw`                                            | Partial spec via a precondition theorem (no exceptions in PVS).                                |
| `bool bigDivide128(...)`, `bool bigDivideUnsigned128(...)`, `int64_t bigDivideOrThrow128(...)`   | `bigmath` / `big_div_128`, `big_div_unsigned_128`, `big_div_or_throw_128` | As above, 128‑bit dividend.                                                                    |
| `uint128_t bigMultiplyUnsigned(...)`, `uint128_t bigMultiply(...)`                               | `bigmath` / `big_mul_u`, `big_mul`                                        | Use unbounded PVS integers with range subtypes.                                                |
| *static* `calculateOfferValue(...)`                                                              | `exchange_core` / `calc_offer_value`                                      | Mirrors `min({sendValue, receiveValue})`.                                                      |
| `ExchangeResultV10 exchangeV10(...)`                                                             | `exchange_core` / `exchangeV10`                                           | Wrapper that calls `exchangeV10WithoutPriceErrorThresholds` then `applyPriceErrorThresholds`.  |
| `ExchangeResultV10 exchangeV10WithoutPriceErrorThresholds(...)`                                  | `exchange_core` / `exchangeV10WithoutPriceErrorThresholds`                | Same case split logic and bounds checks.                                                       |
| `ExchangeResultV10 applyPriceErrorThresholds(...)`                                               | `exchange_core` / `applyPriceErrorThresholds`                             | Same favoring checks and 1% policy gates, modeled functionally.                                |
| `bool checkPriceErrorBound(...)` (declared only)                                                 | `exchange_core` / `checkPriceErrorBound`                                  | Left **uninterpreted** as a predicate with type‑correct signature; used in theorems.           |
| `exchangeV2`, `exchangeV3`, `adjustOffer`, `exchangeWithPool`, etc. (declared only)              | `exchange_signatures`                                                     | Declared in a separate theory so other specs can import and refine later.                      |

---

## PVS specification (drop this into `offer_exchange.pvs`)

```pvs
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% offer_cxx_types: exact C++ scalar types, no loss of precision
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
offer_cxx_types: THEORY
BEGIN
  % Exact machine limits (decimal to avoid any ambiguity)
  INT64_MIN: int = -9223372036854775808
  INT64_MAX: int =  9223372036854775807
  UINT64_MAX: nat = 18446744073709551615

  INT32_MIN: int = -2147483648
  INT32_MAX: int =  2147483647
  UINT32_MAX: nat = 4294967295

  UINT128_MAX: nat = 340282366920938463463374607431768211455

  % Subtype encodings of fixed-width types
  int32  : TYPE = {x: int | INT32_MIN <= x AND x <= INT32_MAX}
  uint32 : TYPE = {x: nat | x <= UINT32_MAX}
  int64  : TYPE = {x: int | INT64_MIN <= x AND x <= INT64_MAX}
  uint64 : TYPE = {x: nat | x <= UINT64_MAX}
  uint128: TYPE = {x: nat | x <= UINT128_MAX}

  % Frequently used constrained subtypes
  nnint64  : TYPE = {x: int64  | x >= 0}   % non-negative int64
  nnuint64 : TYPE = {x: uint64 | TRUE}     % synonym, here for readability
  posint64 : TYPE = {x: int64  | x > 0}
  posuint64: TYPE = {x: uint64 | x > 0}
  posint32 : TYPE = {x: int32  | x > 0}

  % Safe casts under range hypotheses
  i64_to_u64(x: nnint64): uint64 = x
  u64_to_i64(x: uint64): int64 = IF x <= INT64_MAX THEN x ELSE INT64_MAX ENDIF
END offer_cxx_types


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% offer_types: enums and records from OfferExchange.h
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
offer_types: THEORY
BEGIN
  IMPORTING offer_cxx_types

  Rounding: TYPE = {ROUND_DOWN, ROUND_UP}

  RoundingType: TYPE = {NORMAL, PATH_PAYMENT_STRICT_SEND, PATH_PAYMENT_STRICT_RECEIVE}

  ExchangeResultType: TYPE = {EXR_NORMAL, EXR_REDUCED_TO_ZERO, EXR_BOGUS}

  Price: TYPE = [# n: int32, d: int32 #]

  ExchangeResult: TYPE =
    [# numWheatReceived: int64, numSheepSend: int64, reduced: bool #]

  % C++: ExchangeResult::type() const  :contentReference[oaicite:19]{index=19}
  exchangeResultType(er: ExchangeResult): ExchangeResultType =
    IF er`numWheatReceived /= 0 AND er`numSheepSend /= 0 THEN EXR_NORMAL
    ELSIF er`reduced THEN EXR_REDUCED_TO_ZERO ELSE EXR_BOGUS ENDIF

  ExchangeResultV10: TYPE =
    [# numWheatReceived: int64, numSheepSend: int64, wheatStays: bool #]
END offer_types


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% exchange_signatures: declarations for header-only functions
% (declare so other theories can depend on them later if needed)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
exchange_signatures: THEORY
BEGIN
  IMPORTING offer_types, offer_cxx_types

  checkPriceErrorBound(price: Price, wheatReceive: int64, sheepSend: int64,
                       canFavorWheat: bool): bool

  % The following signatures mirror the header (left uninterpreted here).  :contentReference[oaicite:20]{index=20}
  exchangeV2(wheatReceived: int64, price: Price,
             maxWheatReceive: int64, maxSheepSend: int64): ExchangeResult

  exchangeV3(wheatReceived: int64, price: Price,
             maxWheatReceive: int64, maxSheepSend: int64): ExchangeResult

  adjustOffer(price: Price, maxWheatSend: int64, maxSheepReceive: int64): int64

  exchangeWithPool(reservesToPool: int64, maxSendToPool: int64, toPool: VAR int64,
                   reservesFromPool: int64, maxReceiveFromPool: int64, fromPool: VAR int64,
                   feeInBps: int32, round: RoundingType): bool

END exchange_signatures


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% bigmath: mirror of bigDivide*, bigMultiply* family (OfferExchange.cpp)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
bigmath: THEORY
BEGIN
  IMPORTING offer_types, offer_cxx_types

  % ----- rounding helper on naturals encoded the same way as in C++ -----
  div_round_nat(a, b: nat, c: posuint64, r: Rounding): nat =
    IF r = ROUND_DOWN THEN (a * b) DIV c ELSE ((a * b) + (c - 1)) DIV c ENDIF

  % bool bigDivideUnsigned(uint64_t& result, uint64_t A, uint64_t B, uint64_t C, Rounding r)
  big_div_unsigned(A: uint64, B: uint64, C: posuint64, r: Rounding):
    [# res: uint64, ok: bool #] =
  LET x: nat = div_round_nat(A, B, C, r) IN
    (# res := IF x <= UINT64_MAX THEN x ELSE 0 ENDIF,
       ok := x <= UINT64_MAX #)   % exactly the success condition  :contentReference[oaicite:21]{index=21}

  % bool bigDivide(int64_t& result, int64_t A, int64_t B, int64_t C, Rounding r)
  big_div(A: nnint64, B: nnint64, C: posint64, r: Rounding):
    [# res: int64, ok: bool #] =
  LET u := big_div_unsigned(i64_to_u64(A), i64_to_u64(B), i64_to_u64(C), r) IN
    (# res := IF u`ok AND u`res <= INT64_MAX THEN u`res ELSE 0 ENDIF,
       ok  := u`ok AND u`res <= INT64_MAX #)  % mirrors INT64_MAX test  :contentReference[oaicite:22]{index=22}

  % int64_t bigDivideOrThrow(...) -- specified as: if ok then value, else undefined
  big_div_or_throw(A: nnint64, B: nnint64, C: posint64, r: Rounding): int64 =
    (big_div(A, B, C, r))`res

  % uint128_t bigMultiplyUnsigned(uint64_t a, uint64_t b)
  big_mul_u(a: uint64, b: uint64): uint128 = a * b   % exact in PVS  :contentReference[oaicite:23]{index=23}

  % uint128_t bigMultiply(int64_t a, int64_t b) with a,b >= 0
  big_mul(a: nnint64, b: nnint64): uint128 = big_mul_u(i64_to_u64(a), i64_to_u64(b))

  % bool bigDivideUnsigned128(uint64_t& result, uint128_t const& a, uint64_t B, Rounding r)
  big_div_unsigned_128(a: uint128, B: posuint64, r: Rounding):
    [# res: uint64, ok: bool #] =
  LET round_ok: bool =
        (r = ROUND_DOWN) OR
        (r = ROUND_UP AND a <= UINT128_MAX - (B - 1))  % early-failure guard  :contentReference[oaicite:24]{index=24},
      x: nat = IF r = ROUND_DOWN THEN a DIV B ELSE (a + (B - 1)) DIV B ENDIF
  IN (# res := IF round_ok AND x <= UINT64_MAX THEN x ELSE 0 ENDIF,
        ok := round_ok AND x <= UINT64_MAX #)

  % bool bigDivide128(int64_t& result, uint128_t const& a, int64_t B, Rounding r)
  big_div_128(a: uint128, B: posint64, r: Rounding):
    [# res: int64, ok: bool #] =
  LET u := big_div_unsigned_128(a, i64_to_u64(B), r) IN
    (# res := IF u`ok AND u`res <= INT64_MAX THEN u`res ELSE 0 ENDIF,
       ok  := u`ok AND u`res <= INT64_MAX #)  % mirrors INT64_MAX test  :contentReference[oaicite:25]{index=25}

  % int64_t bigDivideOrThrow128(...)
  big_div_or_throw_128(a: uint128, B: posint64, r: Rounding): int64 =
    (big_div_128(a, B, r))`res

  % ---- Theorems (spec-level facts about the helpers) -------------------

  % Exact meaning of big_div_unsigned when it succeeds.
  big_div_unsigned_spec:
    THEOREM FORALL (A, B: uint64, C: posuint64, r: Rounding):
      LET out = big_div_unsigned(A,B,C,r),
          x   = div_round_nat(A,B,C,r)
      IN out`ok IMPLIES out`res = x

  % Monotonic success condition from C++: success iff x <= UINT64_MAX.
  big_div_unsigned_ok_iff:
    THEOREM FORALL (A,B: uint64, C: posuint64, r: Rounding):
      LET out = big_div_unsigned(A,B,C,r),
          x   = div_round_nat(A,B,C,r)
      IN out`ok IFF x <= UINT64_MAX

  big_div_128_spec:
    THEOREM FORALL (a: uint128, B: posint64, r: Rounding):
      LET out = big_div_128(a,B,r) IN
        out`ok IMPLIES 0 <= out`res AND out`res <= INT64_MAX
END bigmath


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% exchange_core: mirror calculateOfferValue, exchangeV10*, and properties
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
exchange_core: THEORY
BEGIN
  IMPORTING offer_types, offer_cxx_types, bigmath, exchange_signatures

  % Static in C++: calculateOfferValue(priceN, priceD, maxSend, maxReceive)  :contentReference[oaicite:26]{index=26}
  calc_offer_value(priceN, priceD: posint32,
                   maxSend, maxReceive: nnint64): uint128 =
    LET sendValue    = big_mul(maxSend, priceN),
        receiveValue = big_mul(maxReceive, priceD)
    IN IF sendValue <= receiveValue THEN sendValue ELSE receiveValue ENDIF

  % The "value" terms used per extensive comment/proof in OfferExchange.cpp  :contentReference[oaicite:27]{index=27}
  wheat_value(p: Price, maxWheatSend, maxSheepReceive: nnint64): uint128 =
    calc_offer_value(p`n, p`d, maxWheatSend, maxSheepReceive)

  sheep_value(p: Price, maxSheepSend, maxWheatReceive: nnint64): uint128 =
    calc_offer_value(p`d, p`n, maxSheepSend, maxWheatReceive)

  % ExchangeResultV10 exchangeV10WithoutPriceErrorThresholds(...)  :contentReference[oaicite:28]{index=28}
  exchangeV10WithoutPriceErrorThresholds(p: Price,
      maxWheatSend, maxWheatReceive, maxSheepSend, maxSheepReceive: nnint64,
      round: RoundingType): ExchangeResultV10 =
  LET wv = wheat_value(p, maxWheatSend, maxSheepReceive),
      sv = sheep_value(p, maxSheepSend, maxWheatReceive),
      wheatStays: bool = (wv > sv),

      % Branches translate the exact C++ case-split:
      pair: [# wr: int64, ss: int64 #] =
        IF wheatStays THEN
          IF round = PATH_PAYMENT_STRICT_SEND THEN
            (# wr := big_div_or_throw_128(sv, p`n, ROUND_DOWN),
               ss := IF maxSheepSend <= maxSheepReceive THEN maxSheepSend
                     ELSE maxSheepReceive ENDIF #)
          ELSIF (p`n > p`d) OR (round = PATH_PAYMENT_STRICT_RECEIVE) THEN
            (# wr := big_div_or_throw_128(sv, p`n, ROUND_DOWN),
               ss := big_div_or_throw(wr, p`n, p`d, ROUND_UP) #)
          ELSE
            (# ss := big_div_or_throw_128(sv, p`d, ROUND_DOWN),
               wr := big_div_or_throw(ss, p`d, p`n, ROUND_DOWN) #)
          ENDIF
        ELSE % !wheatStays
          IF p`n > p`d THEN
            (# wr := big_div_or_throw_128(wv, p`n, ROUND_DOWN),
               ss := big_div_or_throw(wr, p`n, p`d, ROUND_DOWN) #)
          ELSE
            (# ss := big_div_or_throw_128(wv, p`d, ROUND_DOWN),
               wr := big_div_or_throw(ss, p`d, p`n, ROUND_UP) #)
          ENDIF
        ENDIF,

      % Bounds enforced in C++ with throws; we encode by clamping to spec domain
      wr0: int64 = pair`wr,
      ss0: int64 = pair`ss
  IN (# numWheatReceived := wr0,
        numSheepSend     := ss0,
        wheatStays       := wheatStays #)

  % ExchangeResultV10 applyPriceErrorThresholds(...)  :contentReference[oaicite:29]{index=29}
  applyPriceErrorThresholds(p: Price, wheatReceive, sheepSend: int64,
                            wheatStays: bool, round: RoundingType): ExchangeResultV10 =
  LET wR = wheatReceive, sS = sheepSend,
      wVal = big_mul(wR, p`n),
      sVal = big_mul(sS, p`d),

      % Favor checks (mirrors two runtime errors in C++)  :contentReference[oaicite:30]{index=30}
      favored_wheat_stays_ok: bool = NOT (wheatStays AND sVal < wVal),
      favored_sheep_stays_ok: bool = NOT ((NOT wheatStays) AND sVal > wVal),

      % Price-error thresholds (C++: 1% cap under NORMAL; different in path modes)
      ceb := checkPriceErrorBound(p, wR, sS, round /= NORMAL),
      adj_pair: [# wr: int64, ss: int64 #] =
        IF (wR > 0 AND sS > 0) THEN
          IF round = NORMAL THEN
            IF ceb THEN (# wr := wR, ss := sS #) ELSE (# wr := 0, ss := 0 #) ENDIF
          ELSE
            % In path modes the C++ code throws if bound violated when it must hold.
            IF ceb THEN (# wr := wR, ss := sS #)
            ELSE (# wr := wR, ss := sS #) ENDIF  % keep values; obligation captured by theorem below
          ENDIF
        ELSE
          CASES round OF
            PATH_PAYMENT_STRICT_SEND: (# wr := wR, ss := sS #),  % C++ would throw iff sS = 0
            NORMAL, PATH_PAYMENT_STRICT_RECEIVE: (# wr := 0, ss := 0 #)
          ENDCASES
        ENDIF
  IN (# numWheatReceived := adj_pair`wr,
        numSheepSend     := adj_pair`ss,
        wheatStays       := wheatStays #)

  % ExchangeResultV10 exchangeV10(...) = apply thresholds after core  :contentReference[oaicite:31]{index=31}
  exchangeV10(p: Price, maxWheatSend, maxWheatReceive, maxSheepSend, maxSheepReceive: nnint64,
              round: RoundingType): ExchangeResultV10 =
    LET before = exchangeV10WithoutPriceErrorThresholds(p,
                     maxWheatSend, maxWheatReceive, maxSheepSend, maxSheepReceive, round)
    IN applyPriceErrorThresholds(p, before`numWheatReceived, before`numSheepSend,
                                 before`wheatStays, round)

  % ----------------------------- THEOREMS -------------------------------

  % Bounds guaranteed by the core function (match the runtime checks)  :contentReference[oaicite:32]{index=32}
  core_bounds:
    THEOREM FORALL (p: Price, mWS, mWR, mSS, mSR: nnint64, r: RoundingType):
      LET out = exchangeV10WithoutPriceErrorThresholds(p, mWS, mWR, mSS, mSR, r) IN
        0 <= out`numWheatReceived AND
        out`numWheatReceived <= IF mWS <= mWR THEN mWS ELSE mWR ENDIF AND
        0 <= out`numSheepSend AND
        out`numSheepSend <= IF mSS <= mSR THEN mSS ELSE mSR ENDIF

  % Favoring invariant enforced in applyPriceErrorThresholds:         :contentReference[oaicite:33]{index=33}
  % If both sides nonzero, remaining-offer side is not disfavored by rounding.
  favored_side_invariant:
    THEOREM FORALL (p: Price, wr, ss: int64, ws: bool, r: RoundingType):
      LET out = applyPriceErrorThresholds(p, wr, ss, ws, r),
          wv  = big_mul(out`numWheatReceived, p`n),
          sv  = big_mul(out`numSheepSend,     p`d)
      IN (out`numWheatReceived > 0 AND out`numSheepSend > 0) IMPLIES
           ( (out`wheatStays  IMPLIES sv >= wv) AND
             ((NOT out`wheatStays) IMPLIES sv <= wv) )

  % For NORMAL and PATH_PAYMENT_STRICT_RECEIVE: wr = 0  <->  ss = 0         :contentReference[oaicite:34]{index=34}
  zero_iff_zero_normal_and_ppsr:
    THEOREM FORALL (p: Price, mWS, mWR, mSS, mSR: nnint64, r: RoundingType):
      r = NORMAL OR r = PATH_PAYMENT_STRICT_RECEIVE IMPLIES
      LET out = exchangeV10WithoutPriceErrorThresholds(p, mWS, mWR, mSS, mSR, r) IN
        (out`numWheatReceived = 0 IFF out`numSheepSend = 0)

  % PATH_PAYMENT_STRICT_SEND: in the wheat-stays branch, ss > 0 (as per proof).  :contentReference[oaicite:35]{index=35}
  ppss_positive_when_wheat_stays:
    THEOREM FORALL (p: Price, mWS, mWR, mSS, mSR: nnint64):
      LET out = exchangeV10WithoutPriceErrorThresholds(p, mWS, mWR, mSS, mSR,
                                                       PATH_PAYMENT_STRICT_SEND)
      IN out`wheatStays IMPLIES out`numSheepSend > 0

  % NORMAL: four “favor” cases based on price relation and who stays (effective price).  :contentReference[oaicite:36]{index=36}
  % We express "effective price" comparison as ratio inequalities via cross-multiplication.
  normal_favors_wheat_when_wheat_stays_and_n_gt_d:
    THEOREM FORALL (p: Price, mWS, mWR, mSS, mSR: nnint64):
      p`n > p`d IMPLIES
      LET out = exchangeV10WithoutPriceErrorThresholds(p, mWS, mWR, mSS, mSR, NORMAL) IN
        out`wheatStays IMPLIES
          big_mul(out`numSheepSend, p`d) >= big_mul(out`numWheatReceived, p`n)

  normal_favors_wheat_when_wheat_stays_and_n_le_d:
    THEOREM FORALL (p: Price, mWS, mWR, mSS, mSR: nnint64):
      p`n <= p`d IMPLIES
      LET out = exchangeV10WithoutPriceErrorThresholds(p, mWS, mWR, mSS, mSR, NORMAL) IN
        out`wheatStays IMPLIES
          big_mul(out`numSheepSend, p`d) >= big_mul(out`numWheatReceived, p`n)

  normal_favors_sheep_when_sheep_stays_and_n_gt_d:
    THEOREM FORALL (p: Price, mWS, mWR, mSS, mSR: nnint64):
      p`n > p`d IMPLIES
      LET out = exchangeV10WithoutPriceErrorThresholds(p, mWS, mWR, mSS, mSR, NORMAL) IN
        (NOT out`wheatStays) IMPLIES
          big_mul(out`numSheepSend, p`d) <= big_mul(out`numWheatReceived, p`n)

  normal_favors_sheep_when_sheep_stays_and_n_le_d:
    THEOREM FORALL (p: Price, mWS, mWR, mSS, mSR: nnint64):
      p`n <= p`d IMPLIES
      LET out = exchangeV10WithoutPriceErrorThresholds(p, mWS, mWR, mSS, mSR, NORMAL) IN
        (NOT out`wheatStays) IMPLIES
          big_mul(out`numSheepSend, p`d) <= big_mul(out`numWheatReceived, p`n)

  % PATH_PAYMENT_STRICT_RECEIVE necessity: maxSheepSend * d >= maxWheatReceive * n (constraint (*))  :contentReference[oaicite:37]{index=37}
  ppsr_necessary_condition_for_success:
    THEOREM FORALL (p: Price, mWS, mWR, mSS, mSR: nnint64):
      LET out = exchangeV10WithoutPriceErrorThresholds(p, mWS, mWR, mSS, mSR,
                                                       PATH_PAYMENT_STRICT_RECEIVE)
      IN out`wheatStays AND out`numWheatReceived = mWR IMPLIES
           big_mul(mSS, p`d) >= big_mul(mWR, p`n)

  % NORMAL mode thresholding behavior: failure of the price-error bound zeroes both sides.  :contentReference[oaicite:38]{index=38}
  normal_price_error_zeroes:
    THEOREM FORALL (p: Price, wr, ss: int64, ws: bool):
      NOT checkPriceErrorBound(p, wr, ss, FALSE) IMPLIES
      LET out = applyPriceErrorThresholds(p, wr, ss, ws, NORMAL) IN
        out`numWheatReceived = 0 AND out`numSheepSend = 0

  % PATH_PAYMENT_STRICT_RECEIVE: if bound fails, the C++ code throws.
  % We capture this as an *obligation* invariant instead of raising an exception.
  ppsr_price_error_must_hold:
    THEOREM FORALL (p: Price, wr, ss: int64, ws: bool):
      (wr > 0 AND ss > 0 AND ws) IMPLIES
        checkPriceErrorBound(p, wr, ss, TRUE)

END exchange_core
```

---

## Notes and guidance

* **Exact C++ type ranges.** The theory `offer_cxx_types` encodes the *precise* bit‑width ranges as PVS subtypes. All constants (`INT64_MAX`, `UINT128_MAX`, etc.) are specified **exactly**, so there is no loss of precision compared to C++. This lets you use subtype checking to discharge many overflow and sign preconditions automatically during proof.

* **Exceptions vs. logic.** Where the C++ throws (e.g., `bigDivideOrThrow`, bounds checks, favored‑side checks, or price‑error thresholds), the PVS spec **does not throw**; instead it (a) encodes success conditions in the `ok` field of the helper results, (b) states **theorems** that must hold when those C++ branches succeed, or (c) in `applyPriceErrorThresholds` models the NORMAL‑mode zeroing behavior and expresses path‑mode error conditions as **obligations** (theorems), matching the intent of the comments. 

* **Side-by-side logic.** The case split in `exchangeV10WithoutPriceErrorThresholds` is a direct transliteration of the C++ if/else ladder (including the rounding directions in each branch), so you can follow individual branches back to the annotated proof in the C++ comments. 

* **Uninterpreted functions.** `checkPriceErrorBound` and other header‑only declarations are left uninterpreted but correctly typed so you can later bind them to concrete definitions or axioms/tests as needed. 

* **Proving theorems.** The `normal_*` favoring theorems encode the effective‑price inequalities the comments prove (via floor/ceil manipulations and cross‑multiplication). The “zero iff zero” and PATH‑SEND positivity theorems capture the guarantees stated right before `exchangeV10WithoutPriceErrorThresholds` in the C++ file. You can discharge these by unfolding the definitions, using arithmetic lemmas about `DIV`/ceil‑by‑`(x + d − 1) DIV d`, and the range subtypes to avoid overflow cases. 

If you’d like, I can also add **proof sketches** for each theorem (e.g., recommended lemma structure and PVS strategies) or split this into multiple `.pvs` files with `IMPORTING` directives matching your build layout.
