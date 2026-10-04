# Heritable construction architecture (S4 and S5)

## Authority and implementation status

This records the user-approved S1–S5 architecture in [issue #21](https://github.com/ElliottChambon/AlienEvolution/issues/21). It freezes causal/interface commitments, not scientific mechanism choices, numerical solvers, or an inheritance chemistry. No mature implementation or biological validation is claimed. See the [commitment registry](MATURE_COMMITMENTS.md), [prototype debt](SCIENTIFIC_DEBT.md), and [M0–M7+ migration sequence](ROADMAP.md#m0m7-mature-architecture-migration).

## Approved S4 principle: evolution changes Heritable Construction State

The mature conceptual object is **Heritable Construction State** `G`.

`G` means whatever physically heritable information/state determines how descendants construct/regulate themselves. It must not architecturally require DNA or one Earth inheritance chemistry.

The mature causal chain is:

```text
Heritable Construction State G
-> development D
-> local material state M(x,t) + geometry/topology Omega(t)
-> Material Compiler H
-> derived constitutive/physical-response models C_k
-> Physical Coupling Processes P_j
-> regulation/physiology/behavior/ecology
-> reproduction
```

Mature evolution acts primarily on `G -> G'`, not directly on finished properties such as stiffness, conductivity, absorption peak, sensor gain, chemical affinity, leg length, eye count, etc.

Direct mutation of effective parameters remains permitted ONLY as an explicit reduced experiment/prototype with documented scope and scientific debt, such as V0.3B6.

Development is an essential coupled dynamical layer, not an optional translation from heredity to geometry.
The architecture must permit developmental bias, robustness, pleiotropy, epistasis, neutral changes, nonlinear changes, failures, and mechanochemical feedback.

---

## S4 material architecture

Mature materials must not be bags of independent scalar traits.

Conceptual material state:
```text
M(x,t) =
  composition
  + phase
  + microstructure
  + orientation
  + interfaces
  + internal state
  + relevant history
```

Effective properties should be derived through constitutive/response models as needed.

Material behavior may be:

- anisotropic,
- nonlinear,
- history-dependent,
- frequency-dependent,
- state-dependent,
- multiphysics.

The mature **Material Compiler** derives/cache-validates usable physical response representations from developed material state.

Approved computational strategies include:

- hierarchy from cheap estimates/bounds through coarse-grained/molecular/QM reference models as needed,
- local lineage chemistry frontiers,
- lazy reaction-network growth,
- group/property estimators only within validated domains,
- rigorous material-property bounds,
- representative volume elements where scientifically valid,
- material atlases/caches,
- local mutation-effect compilation with trust regions,
- self-assembly rather than hereditary micromanagement,
- automatic re-evaluation of coupling families after material/development changes.

Caches/compiled models are simulator accelerators and are NEVER inherited biological state.

---

## S4 mutational accessibility / encoding-bias audit

The architecture must explicitly recognize that the representation of `G` and development changes which phenotypes are mutationally accessible.

AlienEvolution must not mistake encoding/developmental bias for a universal law of evolution.

Future validation should include neutral mutation sampling around important `G` states to estimate:
```text
P(delta phenotype | G)
```
including:

- neutral fraction,
- lethal/failure fraction,
- phenotype covariance,
- accessibility of novelty,
- effect-size distributions,
- systematic representation biases.

---

## Approved S5: Heritable Construction Transformation System

The mature transformation system is conceptually:

```text
H = (G, I, M, P)
```

where:

- `G`: Heritable Construction State.
- `I`: inheritance/replication/transmission mechanism.
- `M(de | G, I, W)`: mutation/event kernel producing physically possible inheritance events.
- `P`: simulator-owned immutable evolutionary provenance DAG.

An event is sampled from physically justified opportunities, then applies a mutation footprint:
```text
e ~ M
G' = T_e(G)
provenance records event history
```

The active inheritance mechanism determines which event classes are physically possible.

Do not universalize Earth-DNA mutation mechanics to all alien inheritance systems.

Possible mechanism-dependent transformations may include:

- quantitative/local modification,
- duplication/amplification,
- deletion,
- divergence,
- rearrangement/recombination,
- fusion/splitting,
- relocation/redeployment,
- co-option,
- de novo origin from previously nonfunctional/inert hereditary substrate,
- mobile/self-copying hereditary elements when physically supported,
- horizontal acquisition when physically/ecologically compatible,
- future higher-level symbiotic/inheritance transitions.

Structural events may be neutral, deleterious, or adaptive. Mutation does not ask whether an event is useful before it occurs.

---

## S5 duplication and novelty rules

Duplication does not imply innovation.

Allowed downstream outcomes include:

- dosage effects,
- neutral retention,
- loss/pseudogenization,
- subfunctionalization,
- neofunctionalization,
- divergence,
- specialization.

Latent/incidental function may precede duplication.

Spatial/temporal redeployment of existing construction machinery is a fundamental route to innovation.

Do NOT make mature retargeting an ID mutation such as:
```text
targetId = randomTarget
signalId = randomSignal
```

Mature rewiring/retargeting should ultimately emerge from altered recognition, compatibility, localization, timing, regulation, material state, or other physical/developmental changes.

---

## S5 mutation footprints, no repair, and physically defined opportunities

Every structural event has a **mutation footprint**: the actual hereditary region/components physically copied/changed/deleted/rearranged by the event.

Do not implement generic rules such as:

- duplicate a node and silently clone all graph edges,
- delete a target and automatically retarget a sensor,
- remove a sensor because its regulatory target disappeared,
- repair broken developmental interfaces automatically.

Downstream consequences are re-derived through development/physics and selection.

Mutation opportunity/hazard models must correspond to physical inheritance opportunities, not the combinatorial size of a software graph.

A mature hazard may conceptually follow:
```text
lambda_i = mu_i * N_i(G)
```
where `N_i(G)` is a physically meaningful opportunity measure, not "all mathematically possible graph edges."

---

## S5 separate identities

Mature architecture MUST keep separate:

1. **historical/provenance identity**
2. **current construction identity/state**
3. **current physical/functional role**

Function does not define ancestry.
Ancestry does not guarantee current function.

Do not attach permanent biological labels such as `isSensor`, `Photoreceptor`, `Eye`, etc. to heritable elements.

A sense is diagnosed after development/physics as an informational causal pathway.

---

## S5 provenance and dependency graphs

The **provenance DAG**:

- is immutable simulator bookkeeping,
- records descent, duplication, recombination/fusion, horizontal acquisition, de novo origin, etc.,
- has no physical influence on organisms,
- supports historical relationships/homology,
- may require multiple parents and therefore must not be restricted to a tree.

Separately, maintain a **derived causal dependency graph** for computation:
```text
heritable element
-> developmental product/state
-> material region
-> geometry
-> constitutive response
-> PCP
-> functional/ecological consequences
```

It enables pleiotropy-aware incremental invalidation/recomputation.

The dependency graph is NOT inherited biological information.

---

## S5 scientific distinctions

Record separately:

- **structural novelty**: a new/changed hereditary structure exists,
- **functional innovation**: development/physics now produces a causal capability absent from the ancestor,
- **adaptation**: selection increases frequency/performance because of reproductive consequences.

Do not label a mutation event itself as "gain vision" or "novel sensor created."

Novelty is diagnosed after development and physics.

Constructive neutral evolution and developmental-system drift must remain possible.

Modules are not predefined anatomy/function labels; modularity may emerge and may later be detected/compressed for analysis/performance.

Inheritance machinery and mutation spectra are eventually evolvable. Do not reward evolvability directly; normal lineage consequences determine its fate.

---

## Computational sharing and incremental recomputation

Compiled response models, material atlases, caches, and causal dependency graphs belong to the simulator. They may share reusable calculations within certified domains, but are never inherited construction state. Development/material changes invalidate dependent constitutive responses, PCPs, and functional/ecological consequences; local mutation-effect compilation is usable only inside its validated trust region. Re-evaluate coupling families after relevant changes so pruning cannot remove future evolutionary possibilities. Dependency tracking must preserve pleiotropic effects across regions and functions.

The symbols are local to their contracts: Material Compiler H differs from transformation system H = (G, I, M, P); material state M(x,t) differs from mutation kernel M(de | G,I,W); constitutive responses C_k differ from PCP control C. W denotes the relevant world context of the physically justified inheritance opportunities. These conceptual symbols do not choose concrete class layouts or mechanism backends.

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

Issue #21 supplies the approved architecture. The literature below records supporting physical/material and terrestrial evolutionary contexts; it does not authorize DNA as universal inheritance, select mutation kernels, or calibrate alien novelty rates. Mechanism-specific review, uncertainty, versions, validity domains, and benchmarks remain required before implementation freezes a backend.

| Topic / literature | Scope and limitation |
| --- | --- |
| [Developmental Bias and Evolution: A Regulatory Network Perspective (2018)](https://academic.oup.com/genetics/article/209/4/949/5930979) | Genotype–phenotype maps and biased accessible variation; representation bias must be audited. |
| [Gene Co-Option in Physiological and Morphological Evolution (2002)](https://doi.org/10.1146/annurev.cellbio.18.020402.140619), [On the individuality of gene regulatory networks (2021)](https://pmc.ncbi.nlm.nih.gov/articles/PMC8382235/) | Regulatory redeployment and context-dependent network reuse, not random target-ID mutation. |
| Wegst et al., [Bioinspired structural materials (2015)](https://doi.org/10.1038/nmat4089) | Hierarchical material organization; terrestrial components are evidence, not required chemistry. |
| Kanit et al., [Representative volume element size (2003)](https://doi.org/10.1016/S0020-7683(03)00143-4) | Validity and precision of coarse-grained effective properties; not independent scalar material traits. |
| [The evolution of gene duplications: classifying and distinguishing between models (2010)](https://www.nature.com/articles/nrg2689) | Multiple retention/divergence routes and assumptions; duplication does not imply innovation. |
| Van Oss and Carvunis, [De novo gene birth (2019)](https://doi.org/10.1371/journal.pgen.1008160) | Origin from previously non-genic substrate in DNA systems; other inheritance requires its own physical model. |
| [Diversity and consequences of structural variation in the human genome (2025)](https://www.nature.com/articles/s41576-024-00808-9) | Heterogeneous mutation mechanisms and consequences; not graph-size opportunity counts. |
| [Horizontal gene transfer: building the web of life (2015)](https://www.nature.com/articles/nrg3962) | Transfer and multiple historical sources; physical/ecological compatibility remains necessary. |
| [Constructive Neutral Evolution 20 Years Later (2021)](https://doi.org/10.1007/s00239-021-09996-y) | Neutral routes to complexity must remain possible; structural events are not inherently adaptive. |
| [Understanding developmental system drift (2024)](https://pmc.ncbi.nlm.nih.gov/articles/PMC11529278/) | Conserved traits can have divergent developmental underpinnings; ancestry and function differ. |
| [Mutational robustness and the role of buffer genes in evolvability (2024)](https://doi.org/10.1038/s44318-024-00109-1), [Evolution of evolvability in rapidly adapting populations (2024)](https://www.nature.com/articles/s41559-024-02527-0) | Inheritance/mutation spectra can evolve through lineage consequences; no direct evolvability reward is authorized. |

These contexts can contain demonstrated biological outcomes alongside unresolved mechanisms; assign evidence statuses per layer using [P1/B1/B2/M/X/S](SENSING_FOUNDATIONS.md#approved-evidenceprovenance-categories). Do not substitute one confidence percentage for this record.
