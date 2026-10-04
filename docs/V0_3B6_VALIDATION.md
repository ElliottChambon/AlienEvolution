# V0.3B6 sensory-response selection validation

## Question and preregistered prediction

With internal regulation and sensor wiring fixed, does selection drive inherited
resource sensitivity along the locally fitness-improving direction relative to
matched neutral controls? The causal chain is resource -> sensory transduction ->
internal regulatory output -> deposition -> material/boundary -> energetic fitness
-> reproduction -> inherited sensory response. This is architecture/mechanism
verification, not empirical biological calibration or alien predictive accuracy.

The deterministic landscape was generated before any stochastic B6 evolution.
The fixture below is frozen before the full run. No stochastic pilot, parameter
adjustment, or result-driven retuning is used. The benchmark cannot be made to
pass by changing parameters after viewing full evolutionary outcomes.

## Literature-informed rationale

Literature motivates experimental logic, not the numerical constants below:

- [Valencia-Montoya, Pierce & Bellono (2024), Evolution of Sensory Receptors](https://doi.org/10.1146/annurev-cellbio-120123-112853)
  motivates studying inherited properties at the organism/environment interface.
- [Oteiza & Baldwin (2021), Evolution of sensory systems](https://pubmed.ncbi.nlm.nih.gov/34600187/)
  describes sensory diversity and evolutionary contingency. The authors at the
  issue's linked record are Oteiza and Baldwin, rather than Baldwin and Ko.
- [Sourjik & Berg (2002), Receptor sensitivity in bacterial chemotaxis](https://pubmed.ncbi.nlm.nih.gov/11742065/)
  illustrates that sensitivity is a measurable response property. Its particular
  receptor biology and amplification mechanism are not assumed here.
- [Chevin & Lande (2015), Evolution of environmental cues for phenotypic plasticity](https://pubmed.ncbi.nlm.nih.gov/26292649/)
  motivates relating selection on sensitivity to environmental cues and fitness.
- [Travisano & Lenski (1996), Targets of selection and the specificity of adaptation](https://pubmed.ncbi.nlm.nih.gov/8722758/)
  motivates independent replicate histories and post-evolution fitness assays.
- [Mahilkar et al. (2022), Experimental Evolution of Anticipatory Regulation in Escherichia coli](https://pmc.ncbi.nlm.nih.gov/articles/PMC8787300/)
  motivates explicit controls and separation of evolutionary treatment from
  response assays. Anticipatory regulation is not implemented by this fixture.

These are methodological motivations, not a claim that extraterrestrial sensing
must use Earth receptors, cells, organs, or biochemical pathways.

## Exact frozen fixture

All constants are in [the experiment fixture](../experiments/sensory_selection_validation.hpp).
The experiment uses public core APIs and does not change production `Simulation`.

| Setting | Value |
| --- | --- |
| Internal regulatory nodes | One: ID 3, initial activity 0, basal production 0.01, degradation 1 |
| Intrinsic interactions | None; no legacy dummy input nodes |
| Sensory channel | One, signal ID 200 -> target ID 3, fold change 20, cooperativity 2 |
| Founder sensitivity `K0` | 1.0, identical founders without initial variation |
| Runtime local-material/resource signal IDs | 100 / 200; no inherited local-material channel |
| Environment gravity | 9.80665 m/s²; not mechanically evaluated by this prototype |
| Development | 9 × 9 grid, centered material seed 1, six synchronous development steps |
| Neighborhood length scale | 1.0 |
| Regulatory integration | RK4, time step 0.05, ten substeps per development step |
| Output readout | Half-saturation 0.2, cooperativity 2 |
| Deposition rate scale | 0.25; existing resource multiplier and occupancy bounds retained |
| Energetics | Boundary gain 1.0, material maintenance 0.10 |
| Fitness | Existing `max(0, net energy)` |
| Candidate resources | 0.25, 0.5, 1.0, 1.5, 2.0 |
| Sensitivity scan | 41 log-spaced points: `K_i = 0.05 * 100^(i/40)`, `i=0..40` |
| Local probes | `K0 * exp(-0.1)` and `K0 * exp(+0.1)` |
| Effectively zero gradient threshold | Absolute gradient ≤ 1e-8 |
| Replication | 30 matched replicates per treatment, population 30, 100 reproduction transitions |
| Seed schedule | Replicate index `r=0..29`, seed `20261004 + 1009*r` |
| Generation recording | Founder generation 0 through final generation 100, inclusive |
| Sensory mutation | Rate 0.10 events/channel/replication; half-saturation log SD 0.15 |
| All other mutation | Regulatory rates zero; fold-change/cooperativity effect scales zero; no structural sensory changes |
| Generator safety limits | Existing defaults: 1,000,000 events/component/replication |
| Standardized response probes | 0, 0.25, 0.5, 1.0, 2.0 |

The rates/effect sizes are declared computational null-model parameters, not
biological constants. Only `halfSaturation` can vary. Every organism evaluation
checks fixed topology, regulation, channel count, identities, fold change, and
cooperativity; an isolation violation aborts rather than being assigned fitness.

## Phase 1: deterministic landscape and frozen direction

The gradient is `(fitness(K0*exp(0.1)) - fitness(K0*exp(-0.1))) / 0.2`.
Select the viable candidate (founder fitness > 0) with largest absolute nonzero
gradient; exact ties retain candidate order. The predicted direction is its sign.
If an opposite-sign viable candidate exists, also test the strongest such candidate.
If none is identifiable, report that limitation without stochastic tuning.

Actual deterministic scan (values rounded here; CSV stores 17-digit precision):

| Resource | Founder fitness | Lower-K probe | Higher-K probe | Gradient | Grid optimum K | Grid optimum fitness | Boundary |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 0.25 | 0.901078 | 0.901322 | 0.900892 | -0.002154 | 0.05 | 0.957482 | Lower |
| 0.50 | 2.084817 | 2.134417 | 2.045036 | -0.446909 | 0.05 | 3.652062 | Lower |
| 1.00 | 8.033616 | 8.606653 | 7.464046 | -5.713034 | 0.05 | 13.162093 | Lower |
| 1.50 | 20.675213 | 21.632727 | 19.603033 | -10.148468 | 0.05 | 26.908880 | Lower |
| 2.00 | 37.796930 | 38.798544 | 36.616453 | -10.910455 | 0.05 | 43.658368 | Lower |

**Frozen primary environment: resource 2.0; expected direction: -1 (lower K).**
All candidates have negative local gradients. The stronger environment-specific
reversal comparison is unavailable; it is not fabricated. A scan-boundary optimum
is an approximate best sampled point, not proof of a globally optimal sensitivity.

## Phase 2: matched treatments and primary statistic

Each replicate seed is reused across:

1. `selected_mutation`: fitness-proportional parents; sensory K mutation.
2. `neutral_mutation`: uniform parents; identical sensory K mutation.
3. `selected_no_mutation`: fitness-proportional parents; all mutation rates zero.

Seeds match initial conditions, not necessarily subsequent random streams once
parent selection diverges. No-mutation lines remove heritable variation entirely.
No regulatory or structural sensory evolutionary route is available.

Primary endpoint, fixed before evolution:

```text
adaptiveShift = expectedDirection * (meanLogK_final - log(K0))
pairedDifference = adaptiveShift_selected - adaptiveShift_neutral
```

Across matched replicate differences report mean, sample SD, and the earlier
project convention `mean ± 1.96 * SD / sqrt(n)` as an approximate descriptive 95%
mean interval, plus positive/negative/tie counts (absolute tie tolerance 1e-12).
This is not a replacement for inspecting the full pattern or a calibrated coverage
guarantee. Unavailable extinct endpoints are counted explicitly, never imputed as
adaptive successes. Secondary outcomes are paired fitness, no-mutation fitness,
actual-environment modulation, material, and boundary differences.

## Phase 3: independent post-evolution assays

After the final generation, without mutation or RNG draws, assay every final
population's inherited channel response on the fixed probe grid. Re-develop every
final program in the training environment and record per-organism K, fitness,
material, boundary, net energy, and modulation. If an opposite-gradient environment
is available, also assay reciprocally without further evolution. These read-only
assays do not feed back into inheritance or selection.

## Reproduction and output records

Build in Release and run the fast suite first:

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
.\build\Release\AlienEvolutionSensorySelectionValidation.exe --landscape-only
.\build\Release\AlienEvolutionSensorySelectionValidation.exe --full
```

The full command requires the previously generated landscape CSV to match the
recomputed deterministic scan exactly before creating RNGs for evolution. It
refuses to overwrite an existing full-run CSV. A separate output directory may be
specified for a reproducibility replay; numerical/biological parameters cannot be
overridden at runtime. The long run is not in CTest or executed by CI.

Generated files remain ignored under `experiments/output/`:

- `sensory_selection_landscape.csv`: scan and local probes, gradients, optimum/boundary flags, environment selection.
- `sensory_selection_raw.csv`: all generations' population, fitness, K statistics, phenotype/energy, modulation, extinction/failure counts.
- `sensory_selection_runs.csv`: final endpoints and adaptive shift.
- `sensory_selection_response_assay.csv`: read-only response and phenotype assays.
- `sensory_selection_final.csv`: individual final K distribution and re-evaluation measurements.
- `sensory_selection_failures.csv`: any caught development failures; zero reproductive fitness with unavailable phenotype metrics, not invented metric zeros.
- `sensory_selection_summary.csv`: paired primary/secondary summaries with unavailable and sign counts.

Uncaught mutation, assay, isolation, or file errors abort with a nonzero exit;
partial CSVs must not be interpreted as a completed experiment.

## Actual full-run results

Pending the first full evolutionary run. This section will report the actual
outcomes, whether supportive, negative, or ambiguous, without fixture changes.

## Claims and limitations

The benchmark can support or reject a sensory evolutionary mechanism within this
specific simplified model. It does not validate universal receptor biology,
Earth-calibrated mutation, mature sensing specificity, realistic resource transport,
or extraterrestrial prediction. The fixed one-dimensional coordinate, scalar
resource, shifted-Hill response, 2D deposition, boundary-based acquisition,
maintenance, and fixed-size non-overlapping populations constrain its meaning.
Selection may saturate or reverse outside the founder's local landscape; drift
and mutation-selection balance remain relevant to ambiguous outcomes.

The [mature sensing commitment](MATURE_COMMITMENTS.md#6-separate-heritable-sensoryprogram)
remains scaffolded. Structural sensory evolution requires its explicit future
architecture checkpoint. Production simulation/development, physics, and mutation
semantics are not changed to obtain this result.
