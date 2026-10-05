# Chemical coupling architecture and CHEM-1

This records the approved M7/CHEM-1 decisions in
[issue #35](https://github.com/ElliottChambon/AlienEvolution/issues/35), specializing
[S1–S5](PHYSICAL_COUPLING_ARCHITECTURE.md), [sensing foundations](SENSING_FOUNDATIONS.md)
and [Heritable Construction State](HERITABLE_CONSTRUCTION_ARCHITECTURE.md).
M7A implements the analytical and well-mixed scaffold only. Mature chemical sensing
and the full spatial CHEM-1 benchmark are incomplete.

## Causal chain and physical support

The mature chain is environment/material chemical composition and transport
-> physical access at coupling support -> association/reaction and material
state changes -> downstream physical/developmental consequences -> energetic
and functional consequences -> selection. Senses are diagnosed causal/informational
pathways, not primitive named chemical channels.

Physical coupling support is the material region/interface and contact geometry
where access and interaction occur. Accessibility, transport, geometry and material
composition must cause the interaction. Software ports, semantic ligand roles and
signal IDs cannot create access or biological function. The CHEM-1 sphere is an
ideal benchmark geometry, not anatomy. `chem1.ideal_spherical_support` is an inert
schema association, not an implemented geometry framework or runtime connection.

Chemical amount/composition and thermodynamic activity are distinct. Constituent
counts belong to matter accounting; activity reflects chemical potential under
declared thermodynamics. Concentration here is an explicitly ideal-dilute input,
not a universal substitute for activity. Consistent number-based units are `a`
in m, `D` in m²/s, `k_a`/`k_on` in m³/s, `k_d`/`k_off` in 1/s, and `c`/`K_D` in
1/m³. No unit conversion engine is supplied.

Mature coupling requires conservative, bidirectional exchanges: association removes
free constituent and changes reactive material state; dissociation returns it.
Matter/energy bookkeeping and environmental feedback cannot be replaced with
one-way signal copying. An externally maintained reservoir is a declared open-system
reduction. M7A records probability balance only; it does not implement environmental
matter accounting or explicit energy exchange.

## State, reactions, thermodynamics and compiled transitions

Interactions are processes changing physical material state. State/reaction
descriptions must retain the degrees of freedom needed by their QoIs. Markov
models are scoped reductions: discarding spatial rebinding/history does not make
the underlying physics memoryless.

Passive reversible chemistry must respect equilibrium/detailed balance under its
declared thermodynamics. Driven chemistry needs explicit driving and energetic
accounting; independently adjusting effective forward/reverse rates cannot replace
that accounting. CHEM-1A is passive and satisfies
`k_on*c*(1-b_eq) = k_off*b_eq` where defined.

Elementary parameters and compiled effective transitions are distinct. `k_a` and
`k_d` are intrinsic benchmark inputs; `k_on`, `k_off`, rebinding and `K_D` are
compiled ideal-sphere quantities, not new inherited traits or mutation targets.
Material changes must cause altered chemistry through a justified mechanism.
Arbitrary signal/target IDs and permanent biological role labels cannot substitute
for material causality. There is no HCS, reproduction or mutation integration.

## Frozen CHEM-1 problem

`L + S <-> LS` denotes mobile constituent L, generic reactive material state/site
S and associated state LS. No food/toxin roles, receptor classes or smell/taste
ontology are assigned. The canonical mature reference is ideal 3D diffusion with
relative diffusion coefficient `D` to a sphere of radius `a`, finite intrinsic
reactivity `k_a` and intrinsic dissociation `k_d`:

```text
k_D = 4*pi*D*a
chi = k_a/k_D
k_on = k_D*k_a/(k_D+k_a)
p_rebind = k_a/(k_a+k_D)
k_off = k_d*k_D/(k_a+k_D)
K_D = k_d/k_a = k_off/k_on
```

`chi` measures reaction versus diffusion limitation, not fidelity. Small `chi`
approaches `k_on=k_a`; large `chi` approaches `k_on=k_D`; `chi=1` gives `k_D/2`.
These ideal-dilute spherical relations and collapsed rebinding do not describe
arbitrary geometry, dense mixtures or cooperative chemistry. `DiffusiveField2D`
is neither the reference nor a source of literal 3D rates.

The effective Markov rates and collapsed rebinding probability are the intended
reduction when rapid local rebinding can be integrated out and rebinding interference
is negligible. Ideal spherical geometry alone does not guarantee this condition:
spatial history and many-particle interference can invalidate the two-state Markov
description even within ideal-dilute chemistry. This is a declared validity
assumption, not an evaluated criterion or a newly selected timescale threshold.

For an open, well-mixed constant reservoir and one reactive site:

```text
0 --k_on*c--> 1
1 --k_off----> 0
b_eq = c/(K_D+c)
tau_R = 1/(k_on*c+k_off)
db/dt = k_on*c*(1-b)-k_off*b
b(t) = b_eq+(b0-b_eq)*exp[-(k_on*c+k_off)*t]
```

Single-site occupancy is noncooperative; no free Hill coefficient is fitted.
Equal `K_D` can coexist with different response times. Constant-concentration
segments can compose imposed transient forcing; this does not solve reservoir
transport or a time-varying reaction network.

For `k_d>0`, `c=0` gives `b_eq=0`. At the permitted boundary `k_d=0,c=0`, both
hazards vanish and every initial occupancy is stationary. The equilibrium helper
reports `std::domain_error` for this nonunique equilibrium; relaxation is infinite
and deterministic/CTMC dynamics preserve initial state. For `k_d=0,c>0`, binding
is absorbing and equilibrium occupancy is one. These follow the frozen equations.

The frozen canonical spatial benchmark has a reactive sphere at radius `a`.
Its **open/reservoir** configuration maintains external concentration for
association-rate validation. Its **closed finite spherical domain** has a
reflecting outer boundary for depletion/conservation tests. These approved spatial
configurations are non-executable declarations and remain unimplemented in M7A;
the well-mixed reservoir helper does not implement either spatial domain.

The closed configuration must preserve free L plus bound L
and unbound S plus LS. Depletion changes concentration and couples chemistry to
transport and matter accounting. That configuration is a declared challenge,
not executable M7A physics.

## Multidimensional ACP and M1–M3 integration

Representations have capability profiles, not a universal scalar ranking:

| Representation | Spatial | Stochastic | Kinetic/history |
| --- | --- | --- | --- |
| Analytical relations | Ideal sphere compiled into rates | No trajectories | Rate relations only |
| Equilibrium reduction | Collapsed | Mean equilibrium | Discarded |
| Deterministic well-mixed | Collapsed | Mean occupancy | Constant-reservoir Markov relaxation |
| Well-mixed CTMC | Collapsed | Intrinsic binary jumps | Constant-reservoir Markov trajectory |
| Future spatial reference | Explicit support/transport; method unresolved | Spatial stochastic | Rebinding history to be resolved |

Implemented profiles share ideal-dilute thermodynamics, one reversible pair as
chemical detail and no driving. Later models can vary these axes independently.
Numerical refinement within a model is distinct from physical model promotion.
Intrinsic physical fluctuations and sampling uncertainty are distinct from
numerical error and scientific/model-form uncertainty.

M1 provides `alien_evolution.chem1.*` identities/version `chem1a-v1`, physical
inputs, assumptions, provenance and reviewed literature context. P1 physical
interaction evidence does not establish biological sensory use, calibration or
spatial validation. M2 provides model-owned concentration/state/observables and
probability ledger vocabulary for kinetics. Algebraic profiles have no dynamic
state; CTMC state is binary and deterministic state is mean bound probability.
There is no universal chemical quantity enum, runtime port wiring or solver API.

M3 declares QoI-specific validity/reference/error vocabulary: rates, `chi`,
rebinding, `K_D`, equilibrium occupancy where applicable, transient response and
relaxation time for kinetics, stochastic mean/variance for CTMC. The identity
`alien_evolution.chem1.spatial_reference`, version `review-pending`, is an
**unimplemented reference obligation**, not a solver or certification.
Analytical checks apply only to analytically defined QoIs; they do not resolve
omitted spatial physics. Contracts do not evaluate criteria, calculate
errors or select/promote models. Numerical differences do not bound model-form
uncertainty or constitute biological validation.

## Implemented subset and numerical verification

[ReversibleChemicalAssociation](../include/alien_evolution/physics/ReversibleChemicalAssociation.hpp)
implements validated physical parameters, analytical rates, equilibrium, derivative,
exact constant-concentration response and two-state CTMC using caller `ae::Random`.

Inputs are finite with `a,D,k_a>0`, `k_d,c,time>=0` and occupancy in `[0,1]`.
There are no silent input clamps. Scaled products and ratio algebra avoid simple
intermediate overflow; independently calculated probability complements avoid
subtracting nearly one; `expm1` preserves short-time response. Positive derived
rates/ratios outside finite-double range, including underflow to zero, throw
`std::overflow_error`. Finite valid physical inputs do not ensure derived quantities
are representable. Probability ratios can round to zero/one at extreme separation.
Relaxation time can overflow to infinity or underflow to zero at double limits.
An overflowing decay exponent returns the rounded long-time limit. No physical
threshold or equation is changed to repair these cases.

CTMC waits are exponential. Each proposed wait consumes a nonzero uniform draw
(zero is retried); the terminal censored wait consumes its draw. Zero duration
and absorbing states consume none. Events alternate with strictly increasing
times within the horizon. Unrepresentable increments throw; the explicit event
safety limit throws instead of silently truncating. Reproducibility follows the
existing RNG/platform numerical conventions, with no hidden stream or production
RNG changes.

[Fast tests](../tests/reversible_chemical_association_tests.cpp) check input rejection,
chi sweep/limits/crossover, rebinding/effective-rate/K_D consistency, probability
bounds, equilibrium flux balance, exact response/segment composition, identical
affinity with different kinetics and degenerate boundaries. CTMC tests check
seeded trajectories, alternating states, monotonic time, absorbing/no-draw cases
and stationary mean/variance. The predeclared ensemble test uses 12,000 trajectories,
seed 3502, 16 relaxation times and absolute mean/Bernoulli-variance tolerance 0.025.
These are analytical/well-mixed checks, not spatial or biological validation.

## Challenges, expected failures and deferred checkpoint

The M3 registry declares eight spatial comparisons, all **UNRESOLVED**:
reaction-limited, diffusion-limited, transient forcing, low-copy stochastic,
spatial rebinding, finite-domain depletion/conservation, spherical encounter-radius/
support-size variation (association-rate QoI), and near-validity-boundary
cross-fidelity disagreement at the onset of rebinding interference or spatial
memory within ideal-dilute chemistry. The boundary case concerns spatial/kinetic
fidelity, not dilute-to-nondilute thermodynamic promotion: the future ideal-dilute
spatial reference cannot certify nonideal thermodynamics. Nonspherical supports
remain a later extension, separate from the first geometry challenge.
Contexts are inert scenario
descriptions, not reproducible spatial snapshots before solver/configuration
review. Analytical passes do not mark spatial entries passed.

Expected failures include non-Markov rebinding/arrival correlations, closed-domain
depletion, altered support geometry, crowding/nonideal activity, mixtures and
competition, cooperative sites, explicit driving, amplification, adaptation and
proofreading. Mean occupancy cannot reproduce intrinsic fluctuations; equilibrium
cannot reproduce kinetic history. Numerical refinement alone cannot repair these
physical model limitations.

Spatial stochastic reference selection is a separate scientific checkpoint.
Brownian dynamics, GFRD/eGFRD, Smoldyn binding/unbinding radii, RDME, general 3D PDE
and many-particle spatial algorithms are **not chosen or implemented**. The already
reviewed context distinguishes exact/event-driven isolated-pair Green's functions
from finite-step particle methods and their numerical assumptions:
[Kim & Shin (1999)](https://doi.org/10.1103/PhysRevLett.82.1578),
[Andrews & Bray (2004)](https://doi.org/10.1088/1478-3967/1/3/001),
[van Zon & ten Wolde (2005)](https://doi.org/10.1103/PhysRevLett.94.128103).
This implementation does not replace that review or select a method.

Mature sensing, nonideal thermodynamics, mixtures, electrochemistry/redox/protonation,
cooperativity, amplification/adaptation/proofreading, and evolutionary/selection-aware
cross-validation remain [debt](SCIENTIFIC_DEBT.md#chem-1a-analytical-and-well-mixed-scaffold).
Production `Simulation`, `SensoryProgram`, `RegulatoryInputInterface`, development,
mutation/reproduction, B4–B6 and the separate M6 phenomenological PCC benchmark
remain unchanged. M7A completion means a scaffold ready for future spatial
cross-validation, not full CHEM-1 or mature sensing.
