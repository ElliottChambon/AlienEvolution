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

The first full run completed locally on 2026-10-04 UTC (2026-10-03 Central),
Windows/MSVC Release, in **639.934 seconds**, exit code 0. The fixture, landscape,
and prediction were committed before evolution in
`aca7be52a2047996559fbf1f81b645736b5f8c3b`. No stochastic pilot or fixture
adjustment was used. Experiment source hashes remained unchanged after the run.
All 30 matched replicates, three treatments, populations of 30, and 100
reproduction transitions were completed without reductions.

**The primary prediction is supported within this controlled prototype.**
Selected lines moved toward lower K relative to neutral lines, the direct response
curves changed accordingly, and fixed-component checks excluded regulatory and
structural sensory routes. Five of the 30 paired sensory comparisons were negative.
This ensemble result does not establish environment-specific reversal, mature
sensing, or biological calibration.

### Paired endpoints

All rows have 30 available pairs and zero unavailable pairs. Intervals use the
preregistered descriptive convention. Secondary outcomes do not replace the
primary endpoint and are not independently corrected hypothesis tests.

| Endpoint | Mean | Sample SD | Approximate 95% mean interval | Positive / negative / tie |
| --- | ---: | ---: | --- | --- |
| **Adaptive shift, selected minus neutral** | **0.481431** | **0.396876** | **[0.339411, 0.623452]** | **25 / 5 / 0** |
| Adaptive shift, selected | 0.605052 | 0.262091 | [0.511264, 0.698840] | 30 / 0 / 0 |
| Adaptive shift, neutral | 0.123621 | 0.383022 | [-0.013442, 0.260684] | 18 / 12 / 0 |
| Fitness, selected minus neutral | 3.779074 | 3.828118 | [2.409200, 5.148949] | 26 / 4 / 0 |
| Fitness, selected minus no mutation | 3.663539 | 1.091136 | [3.273081, 4.053997] | 30 / 0 / 0 |
| Training modulation, selected minus neutral | 2.132901 | 2.022748 | [1.409070, 2.856732] | 26 / 4 / 0 |
| Material, selected minus neutral | 5.222384 | 5.252100 | [3.342944, 7.101824] | 26 / 4 / 0 |
| Boundary, selected minus neutral | 2.150656 | 2.176654 | [1.371751, 2.929562] | 26 / 4 / 0 |

### Final sensory distributions and phenotype consequences

Values average the 30 final population summaries, except the range, which spans
all 900 final individuals per treatment. Median and quantiles are means of
within-population statistics, not pooled quantiles.

| Final statistic | Selected + mutation | Neutral + mutation | Selected + no mutation |
| --- | ---: | ---: | ---: |
| Mean K | 0.575085 | 1.006566 | 1.000000 |
| Median K | 0.567718 | 0.922336 | 1.000000 |
| Mean log K | -0.605052 | -0.123621 | 0.000000 |
| K 10th percentile | 0.455968 | 0.705230 | 1.000000 |
| K 90th percentile | 0.709313 | 1.361277 | 1.000000 |
| Individual K range | [0.198823, 1.256770] | [0.313324, 7.103111] | [1, 1] |
| Mean fitness / net energy | 41.460469 | 37.681395 | 37.796930 |
| Mean material | 46.537509 | 41.315125 | 41.439781 |
| Mean exposed boundary | 23.057110 | 20.906454 | 20.970454 |
| Mean training modulation | 18.460710 | 16.327809 | 16.200000 |

No-mutation lines retained K = 1 throughout every recorded generation and the
founder phenotype/fitness. In this positive-energy fixture fitness equals net
energy. Neutral arithmetic mean K near 1 does not imply unchanged sensitivity:
its distribution is skewed and its mean log K differs from zero. Evolution is
not constrained to the scan grid; the neutral maximum exceeds its upper bound.
Selected lines did not reach the lower scan boundary or the sampled fitness
maximum. This tests directional response over this duration, not convergence.

### Direct standardized sensory reaction curve

Mean modulation across the 30 final populations, assayed without mutation or RNG:

| Resource probe | Selected + mutation | Neutral + mutation | Selected + no mutation |
| --- | ---: | ---: | ---: |
| 0.00 | 1.000000 | 1.000000 | 1.000000 |
| 0.25 | 4.644137 | 2.852688 | 2.117647 |
| 0.50 | 9.721085 | 6.220728 | 4.800000 |
| 1.00 | 15.285374 | 11.573207 | 10.500000 |
| 2.00 | 18.460710 | 16.327809 | 16.200000 |

Selected populations have stronger responses at every positive probe on average,
while the zero-signal baseline remains 1. Thus the effect is visible in inherited
response, not merely fitness. Re-developing all 2,700 final organisms reproduced
the recorded final population mean fitness within 1e-12. Reciprocal assays are
unavailable because the scan supplied no opposite-gradient environment.

### Failures, verification, and provenance

There were **zero extinct lines and zero development failures** in every treatment;
all recorded populations had 30 organisms. Fixed-component checks never failed.
Outputs contain 205 landscape rows, 9,090 generation rows, 90 final-run rows,
540 assay rows (450 response plus 90 phenotype), 2,700 individual final rows,
eight paired-summary rows, and a failure CSV containing only its header.
An independent PowerShell audit recomputed the primary mean/sample SD/interval
and sign counts, checked complete generation sequences and population/failure
records, verified no-mutation sensitivity, and checked final assay agreement.

The Release build and **20/20 fast CTest tests passed before the full run**.
Tests verify software/numerical behavior; the manual experiment supplies controlled
causal evidence in this prototype, not empirical biology. The full experiment is
not in CTest or CI and its scientific conclusion is not a CI pass criterion.

SHA-256 hashes of actual local artifacts (Windows text line endings):

| File (`experiments/output/`) | SHA-256 |
| --- | --- |
| `sensory_selection_landscape.csv` | `D5484CA2011244EE32A8313C2453354FD641E610C6D9C380548CE1CDFF6CB9C8` |
| `sensory_selection_raw.csv` | `F0DBCB48528BCB1596CC7D908E84105A91B075988FFB584014BD3175D72CFAD1` |
| `sensory_selection_runs.csv` | `FC1FB3710963A80E5F9B89E598D1DC204D93D84B698D4A3E00695448B6FB5E25` |
| `sensory_selection_response_assay.csv` | `948B53B9770F50CE27C03F8ED7156CDED0D4A31A8862435AD09398059B2C52E0` |
| `sensory_selection_final.csv` | `611F1B10AD3D6663382695311F9A4240680B25C3979ADD9888A7F748B47E5BAC` |
| `sensory_selection_failures.csv` | `6E04A9367CEB94235CE81815ADC161C08975111BEBBA9927718ABB4E62E50072` |
| `sensory_selection_summary.csv` | `E071A32FE7CBA14023BCBDE91D4C2C6E53F3DFD1C57079A5397133E6143D0C44` |

CSVs remain locally available and ignored by Git. Same-build replay uses the
declared seed schedule; other platforms may differ in standard-library random
distribution implementation, floating-point roundoff, and file line endings.

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
