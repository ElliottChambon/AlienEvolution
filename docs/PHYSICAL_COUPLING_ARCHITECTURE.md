# Physical coupling architecture (S2)

The approved M7 chemical specialization, M7A analytical/well-mixed CHEM-1
scaffold and M7B exact reversible isolated-pair spatial reference are recorded in
[chemical coupling architecture](CHEMICAL_COUPLING_ARCHITECTURE.md). M7B selects
one reference realization for one SCK/back-reaction benchmark QoI family; it does
not select a universal spatial solver or replace S1–S5 or the production prototype
sensing path. Finite/open-bath, many-particle and general-geometry solver choices
remain later scientific checkpoints.

## Authority and implementation status

This records the user-approved S1–S5 architecture in [issue #21](https://github.com/ElliottChambon/AlienEvolution/issues/21). It freezes causal/interface commitments, not scientific mechanism choices, numerical solvers, or an inheritance chemistry. No mature implementation or biological validation is claimed. See the [commitment registry](MATURE_COMMITMENTS.md), [prototype debt](SCIENTIFIC_DEBT.md), and [M0–M7+ migration sequence](ROADMAP.md#m0m7-mature-architecture-migration).

## Approved S2 principle: Physical Coupling Process is the mature fundamental object

The original one-way Physical Coupling Channel was adversarially tested and rejected as the universal object.

The approved mature fundamental abstraction is a **Physical Coupling Process** (PCP).

A conceptual top-level contract is:

```text
P_{phi,m,Omega}^{[t,t+tau]}
(
  dX,
  dP_out,
  dO,
  dLambda
  |
  H_t,
  P_in,
  C
)
```

Interpretation:

- `phi`: organism-derived physical phenotype/properties; evolution may alter this only through heredity/development.
- `m`: scientific/numerical model; simulator-owned and NEVER evolvable.
- `Omega`: physical region/geometry.
- `H_t`: relevant state/history; permits memory/non-Markovian behavior.
- `P_in`, `P_out`: typed, bidirectional physical ports.
- `C`: internal control arriving from other organismal processes.
- `X`: future physical state/trajectory.
- `O`: accessible physical observables.
- `Lambda`: typed exchange/conservation ledger.

The abstraction must permit:

- deterministic dynamics,
- stochastic dynamics,
- discrete events,
- continuous fields,
- hybrid systems,
- memory/hysteresis,
- multiphysics coupling,
- quantum realizations when genuinely required.

Do NOT force every model into a single classical ODE, GENERIC, or port-Hamiltonian representation.
GENERIC, port-Hamiltonian, bond-graph, stochastic reaction, PDE, stochastic-hybrid, reduced-kernel, and quantum models are possible **realizations under** the PCP interface.

A **Physical Coupling Channel** is retained only as a reduced/compiled special case of a PCP when validated assumptions permit it.

A “sense” is not a PCP object. It is an emergent causal/informational pathway through a network of physical processes.

---

## S2 physical ports and exchange accounting

The mature architecture is bidirectional. Organism and environment may influence each other.

Examples of domain-specific physical-port pairings may include, where scientifically appropriate:

- mechanics: force/traction with motion/velocity/displacement-related variables,
- electrical: potential with current,
- chemistry: chemical potential/activity with species flux,
- thermal: temperature/thermodynamic potential with heat/entropy transfer.

Do not freeze one universal list in code yet.

The exchange ledger `Lambda` should represent actual relevant physical exchanges/consistency terms such as:

- energy,
- linear momentum,
- angular momentum,
- electric charge,
- species/matter,
- entropy production,
- conservation residuals.

Do not create a universal arbitrary `sensorCost`. Construction, maintenance, repair, amplification, emission, and nonequilibrium operation costs must ultimately arise from modeled physical/resource processes.

---

## S2 Adaptive Certified Physics

Adaptive fidelity is a core mature architecture commitment.

The engine should use the cheapest scientifically adequate representation for the current quantity of interest, not a universal low/medium/high fidelity setting. Scientific adequacy is a hard constraint: computational cost may choose among adequate alternatives but must never justify a model whose omitted physics could change the scientific/evolutionary conclusion.

Reference models, runtime physical models and compiled/reduced models are distinct roles. A costly reference may be invoked sparsely to establish or audit validity while routine evolution uses a cheaper certified representation. Cost estimates, cache state and solver choice remain simulator-owned; organisms never inherit or mutate them.

Every reduced model must carry a validity/certification concept including:

- applicability/domain of validity,
- quantity/quantities of interest for which it is validated,
- numerical/reduction error estimate where possible,
- higher-fidelity/reference model,
- model version,
- scientific provenance/evidence,
- assumptions,
- units/parameters,
- uncertainty.

Numerical/reduction uncertainty and scientific/model-form uncertainty MUST remain separate.

High numerical fidelity does not make an uncertain biological mechanism scientifically true.

Scientific uncertainty may be represented as a set of credible alternative models when the literature does not justify probabilities.

Approved computational strategies include:

- conservative detectability/interaction bounds,
- lazy activation of currently relevant couplings,
- response kernels,
- domain-specific reduced physics,
- local high-fidelity refinement,
- statistical rather than microscopic event descriptions,
- asynchronous/multiscale timesteps,
- validated caching/interpolation,
- phenotype/material compilation,
- dependency-driven incremental recomputation,
- lineage-local chemistry/reaction frontiers,
- representative microstructures/RVEs where valid,
- physics/property bounds before detailed solves,
- local compiled mutation-effect models with trust regions,
- optional future ML surrogates only under the same validity/error rules.

Computational pruning may skip a current interaction but may NEVER erase its future evolutionary possibility.

---

## S2 selection-aware fidelity and evolutionary adversarial validation

Evolution is treated as an adversarial optimizer that may discover approximation artifacts.

Fidelity should be promoted when approximation uncertainty can materially alter the scientific/evolutionary result, including reproductive probabilities or relevant phenotype/fitness comparisons.

Approved safeguards:

- validity-boundary promotion,
- novelty-triggered verification,
- elite/high-fitness audits,
- sudden-gain audits,
- random shadow evaluations,
- cross-fidelity evolutionary benchmarks,
- shrinking/repairing validated regions when adversarial cases fail,
- preserving discovered adversarial phenotypes as permanent challenge cases.

No organism may mutate or choose its numerical/scientific solver.

Core rule:
```text
evolution changes phi
simulator controls m
```

---

## PCP networks and interface boundaries

PCPs compose through compatible typed physical ports and accessible observables; internal control can arrive from other organismal processes. State/history remains local to the relevant realization and may include memory or hysteresis. The network must account for relevant bidirectional exchanges and consistency residuals through its typed ledgers. Coupling does not imply that an observable is accessible, informative, integrated, or adaptive. A sensory pathway can cross several PCPs, material regions, and internal processes; no PCP carries a permanent sense identity.

The contract is conceptual, not a selected C++ signature or universal list of port variables. Certification is specific to the quantity of interest and regime. A compiled channel retains its reference PCP, validated assumptions, and promotion/invalidation relationship; compiling it does not turn a reduction into the fundamental physical model.

## Updateability rule

All mature mechanism and architecture records must distinguish:

- stable interfaces/causal commitments,
- replaceable scientific mechanism models,
- unresolved alternatives,
- prototype/reduced abstractions,
- evidence/provenance,
- validity domain,
- model version/supersession.

New literature must be able to replace or add mechanism implementations without requiring the high-level sensing/evolution architecture to be rewritten.

---

## Literature and provenance notes

The approved commitment comes from issue #21; these topic references support its provenance, not a new solver selection. Realization-specific literature review and quantitative validation remain required. Mathematical structure or a successful reduction alone does not establish biological mechanism truth.

| Topic / literature | Scope and limitation |
| --- | --- |
| Grmela and Öttinger, [Dynamics and thermodynamics of complex fluids. I (1997)](https://doi.org/10.1103/PhysRevE.56.6620) | GENERIC thermodynamic structure is one possible realization, not the mandatory PCP language. |
| Mehrmann and Unger, [Control of port-Hamiltonian differential-algebraic systems and applications (2022)](https://arxiv.org/abs/2201.06590) | Multiphysics structure and structure-preserving reduction; applicable model classes must be declared. |
| Peherstorfer, Willcox and Gunzburger, [Survey of Multifidelity Methods in Uncertainty Propagation, Inference, and Optimization (2018)](https://doi.org/10.1137/16M1082469) | Multifidelity methodology; evolutionary safeguards remain project requirements, not a blanket validity certificate. |
| Kanit et al., [Determination of the size of the representative volume element for random composites (2003)](https://doi.org/10.1016/S0020-7683(03)00143-4) | Representative material models require property-specific precision and sampling assumptions. |

[Sensing foundations](SENSING_FOUNDATIONS.md#literature-and-provenance-notes) records sensing-limit, nonequilibrium, and unresolved magnetic-mechanism evidence. Quantum realizations are permitted when required, without selecting a quantum sensing solver. [Benchmarks 12–14](BENCHMARKS.md#benchmark-12--adaptive-fidelity-reduction) retain reduction, promotion, and evolutionary-ordering requirements. Permanent adversarial challenge cases augment that validation; none are implemented by this documentation milestone.
