# Initial power model: quantities and formulas

This reference explains the mathematics of the ideal model in [decision 007](../decisions/007-initial-power-model.md) and the runner/model boundary in [decision 008](../decisions/008-runner-model-boundary.md). It specifies physical quantities, not code, classes, or storage layouts.

## 1. Watts versus watt-hours

**Power is a rate of energy transfer. Energy is an amount.**

- **Watt (W):** how quickly energy is being generated or consumed. A 2 W load consumes energy twice as quickly as a 1 W load.
- **Watt-hour (Wh):** an amount of energy. A constant 1 W load consumes 1 Wh in one hour. A constant 2 W load consumes 1 Wh in half an hour.

Think of stored energy as water in a tank and power as the rate of water flowing in or out. Wh means watts **multiplied by hours**, not watts divided by hours. See the [EIA explanation of power and energy units](https://www.eia.gov/energyexplained/electricity/measuring-electricity.php).

For constant power:

$$
\text{energy in Wh} = \text{power in W} \times \text{duration in hours}
$$

There are 3,600 seconds in an hour, so:

$$
\text{energy in Wh} = \frac{\text{power in W} \times \text{duration in seconds}}{3600}
$$

For example, a 2 W load over 60 seconds consumes:

$$
\frac{2 \times 60}{3600} = \frac{1}{30}\ \text{Wh} \approx 0.033333\ \text{Wh}
$$

In this ideal model, a full 10 Wh battery could supply a constant 2 W load for five hours with no generation. This does not model voltage or a maximum discharge-power limit.

## 2. Quantities entering one model update

| Symbol | Meaning | Unit | Role |
| --- | --- | --- | --- |
| $C$ | Maximum battery energy capacity | Wh | Constant model configuration |
| $P_{\mathrm{base}}$ | Essential/baseline load | W | Constant model configuration |
| $E_0$ | Battery energy at the interval's start | Wh | Starting state supplied by runner |
| $P_g$ | Available generation | W | Changing input supplied by runner |
| $P_p$ | Additional payload draw | W | Changing input; zero when payload is disabled |
| $\Delta t$ | Simulation interval duration | seconds | Supplied by runner |

The following formulas assume $C>0$, $0\leq E_0\leq C$, nonnegative finite power values, and a positive finite duration. Zero-duration behavior and invalid-input error handling remain separate contract choices.

**The formulas describe an interval with constant power inputs.** If generation or load changes within that interval, the separate constant-input portions need their own accounting. Averaging them together can hide a full/empty boundary crossing. This reference does not choose scenario-event timing or a fixed physics-step size.

The initial model assumes 100% efficiency, no self-discharge, and no separate charge/discharge power limits. It reports aggregate shortages without shutting down equipment.

## 3. Demand and unconstrained battery change

Let $h$ be the interval duration in hours:

$$
h = \frac{\Delta t}{3600}
$$

Total requested load power is:

$$
P_L = P_{\mathrm{base}} + P_p
$$

The power balance is:

$$
P_{\mathrm{balance}} = P_g - P_L
$$

Positive balance means potential charging; negative balance means the battery must supply a deficit. Zero means generation exactly matches demand. This is not always the actual battery power: charging stops at full capacity and discharge stops at empty.

Available generation energy and requested load energy over the interval are:

$$
E_g = P_g h
\qquad
E_{\mathrm{req}} = P_L h
$$

Both quantities are in Wh. The hypothetical ending battery energy without capacity boundaries would be:

$$
E_{\mathrm{raw}} = E_0 + E_g - E_{\mathrm{req}}
$$

$E_{\mathrm{raw}}$ is an accounting quantity, not a valid battery state when it falls outside the physical bounds.

## 4. Ending state, curtailment, and unmet demand

Here, $\min(a,b)$ means the smaller value and $\max(a,b)$ means the larger value.

### Ending battery energy

$$
E_1 = \min\left(C,\ \max(0,E_{\mathrm{raw}})\right)
$$

$E_1$ is the returned physical battery state in Wh. It stays between zero and capacity.

### Curtailed energy

$$
E_{\mathrm{curt}} = \max(0,E_{\mathrm{raw}}-C)
$$

This is available surplus input energy that neither loads nor the battery could use. It is not energy removed from the battery and is not automatically converted into electronics heat.

### Unmet demand energy

$$
E_{\mathrm{unmet}} = \max(0,-E_{\mathrm{raw}})
$$

This is requested load energy that generation and the battery could not supply. It does not make the stored energy negative or automatically disable the payload.

### Energy actually supplied to loads

$$
E_{\mathrm{served}} = E_{\mathrm{req}} - E_{\mathrm{unmet}}
$$

These formulas account for a battery filling or emptying partway through a constant-input interval. Clamping $E_1$ alone would omit the excess or shortfall. Under these assumptions, curtailment and unmet demand cannot both be positive in the same constant-input interval.

## 5. Other useful derived values

Ending state of charge is:

$$
\mathrm{SOC}_1 = 100\frac{E_1}{C}\ \%
$$

For example, 6 Wh in a 10 Wh battery is 60%. Charge percentage is derived from energy; it is not another independent physical state.

Net battery energy change is:

$$
\Delta E = E_1-E_0
$$

It is positive for charging and negative for discharging. Separate charge/discharge amounts, if useful, are:

$$
E_{\mathrm{charged}}=\max(0,\Delta E)
\qquad
E_{\mathrm{discharged}}=\max(0,-\Delta E)
$$

These are battery flows; do not add them again as additional sources or losses in the overall balance.

An interval energy can also be expressed as average power:

$$
\overline{P}_{\mathrm{served}}=\frac{3600E_{\mathrm{served}}}{\Delta t}
\qquad
\overline{P}_{\mathrm{unmet}}=\frac{3600E_{\mathrm{unmet}}}{\Delta t}
$$

$$
\overline{P}_{\mathrm{curt}}=\frac{3600E_{\mathrm{curt}}}{\Delta t}
\qquad
\overline{P}_{\mathrm{battery}}=\frac{3600\Delta E}{\Delta t}
$$

All these averages are in W. The battery average uses positive for charging and negative for discharging. **An interval average is not necessarily the instantaneous power at the interval's end.**

## 6. Worked examples

These are illustrative arithmetic cases, not selected spacecraft defaults. All use $C=10$ Wh, $P_{\mathrm{base}}=2$ W, and $\Delta t=1800$ seconds, so $h=0.5$ hours. Every energy column is in Wh.

| Case | Starting energy $E_0$ | Generation $P_g$ (W) | Payload $P_p$ (W) | Ending $E_1$ | Served | Unmet | Curtailed |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Charging | 5 | 4 | 0 | 6 | 1 | 0 | 0 |
| Discharging | 5 | 4 | 4 | 4 | 3 | 0 | 0 |
| Reaches full | 9.5 | 6 | 0 | 10 | 1 | 0 | 1.5 |
| Reaches empty | 0.5 | 0 | 0 | 0 | 0.5 | 0.5 | 0 |
| Empty, generation meets demand | 0 | 2 | 0 | 0 | 1 | 0 | 0 |
| Empty, generation partly meets demand | 0 | 1 | 0 | 0 | 0.5 | 0.5 | 0 |

For the **reaches full** case:

$$
E_g=6\times0.5=3\ \mathrm{Wh},\qquad E_{\mathrm{req}}=2\times0.5=1\ \mathrm{Wh}
$$

$$
E_{\mathrm{raw}}=9.5+3-1=11.5\ \mathrm{Wh}
$$

Only 0.5 Wh fits in the battery. Ending energy is 10 Wh, supplied load energy is 1 Wh, and the remaining 1.5 Wh is curtailed.

For the **reaches empty** case:

$$
E_{\mathrm{req}}=2\times0.5=1\ \mathrm{Wh},\qquad E_{\mathrm{raw}}=0.5-1=-0.5\ \mathrm{Wh}
$$

The battery supplies its available 0.5 Wh; the remaining 0.5 Wh is unmet demand. It empties after $0.5/2=0.25$ hours, or 900 seconds. Supplied power averages 1 W across the whole interval, although supply is 2 W before depletion and zero afterward. This shows why averages must be labeled.

At a much shorter illustrative interval of 10 ms, $\Delta t=0.010$ seconds. A constant 2 W net charging balance adds only:

$$
\frac{2\times0.010}{3600}\approx0.000005556\ \mathrm{Wh}
$$

That small change is expected. Repeated intervals accumulate physical energy changes.

## 7. What the state and report mean

The evolving battery state is stored energy. Capacity and baseline draw are configuration; generation, payload draw, and duration are inputs. The other quantities above are derived values: defining them does not require storing every one independently.

The interval report should make supplied, unmet, and curtailed energy interpretable for that interval. A sensor observation is separate: it samples physical state and carries measurement-time and quality information. An interval accounting report is not automatically a reading at the ending instant.

If run totals are later needed, total each interval's energy, for example:

$$
E_{\mathrm{unmet,total}}=\sum_i E_{\mathrm{unmet},i}
$$

Do not add each interval's ending battery energy to obtain a total: successive values are snapshots of the same store. Likewise, a run's average power is its total energy divided by its total duration, not generally the unweighted average of unequal-duration interval powers.

## 8. Accounting checks

Every valid interval should satisfy, within the chosen numerical tolerance:

$$
E_g+E_0=E_{\mathrm{served}}+E_1+E_{\mathrm{curt}}
$$

$$
E_{\mathrm{req}}=E_{\mathrm{served}}+E_{\mathrm{unmet}}
$$

Also, $0\leq E_1\leq C$, state of charge lies between 0% and 100%, and supplied/unmet/curtailed energy are nonnegative. These identities include full and empty cases, not just ordinary charging and discharging.
