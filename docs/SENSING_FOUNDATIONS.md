# Sensing foundations (S1 and S3)

## Authority and implementation status

This records the user-approved S1–S5 architecture in [issue #21](https://github.com/ElliottChambon/AlienEvolution/issues/21). It freezes causal/interface commitments, not scientific mechanism choices, numerical solvers, or an inheritance chemistry. No mature implementation or biological validation is claimed. See the [commitment registry](MATURE_COMMITMENTS.md), [prototype debt](SCIENTIFIC_DEBT.md), and [M0–M7+ migration sequence](ROADMAP.md#m0m7-mature-architecture-migration).

## Approved S1 principle: sensing starts from physics, not named senses

AlienEvolution must not define a mature fixed catalog such as vision, hearing, smell, touch, gravity sensing, or magnetoreception.

A functional sense may exist only through a causal chain such as:

```text
physical state exists
-> propagation/access
-> organismal material/structure physically couples to it
-> local physical state changes
-> change carries information above noise
-> internal processes use that information
-> development/physiology/behavior/control changes
-> ecological/reproductive consequences follow
```

Distinguish:

- environmental influence,
- physical detectability,
- functionally integrated sensing,
- adaptive relevance.

Environmental influence is a physical effect on the organism, even if no internal
process uses it as information. Physical detectability concerns whether the
accessible change carries distinguishable information above noise under the
relevant physical conditions. Functionally integrated sensing requires internal
processes to use that information and alter control, development, physiology,
or behavior. Adaptive relevance concerns ecological/reproductive consequences
and selection; detectability or integration alone does not establish adaptation.

A physical field may exist without a lineage ever evolving a useful sense.
Sensing may evolve, fail to evolve, degrade, or be lost.

Primitive physical observables are distinct from inferred semantic concepts.
`predator`, `food`, `object`, `distance`, `north`, `location`, etc. are downstream inferences, not primitive sensor channels.

Initial observable families include:

- chemical/electrochemical,
- mechanical,
- thermal,
- electromagnetic radiation,
- electric,
- magnetic,
- ionizing/high-energy particle/radiation,
- internal physical state.

These are world-physics categories, not biological sense enums.

Examples such as sound, humidity, gravity orientation, and infrared thermal detection should be represented through their actual causal physics rather than given universal named-sense primitives.

Sound belongs to mechanical propagation/coupling, rather than an intrinsic
"hearing" input. Humidity involves water thermodynamics and material coupling;
gravity orientation involves gravity/inertia and displacement/loading; infrared
detection involves radiation and subsequent thermal or photochemical coupling.
These are causal examples, not compulsory biological solutions. Internal
physical observables can participate in the same pathways as external ones.

---

## Passive and active sensing

Passive sensing uses physically available incident inputs without an organism-generated probing emission. Active sensing includes organism-generated probing and its propagation, interaction, and return/access pathway. Neither is a named-sense primitive. Emission, construction, maintenance, repair, amplification, and nonequilibrium operation require actual modeled exchanges; a useful return is not guaranteed. Both must satisfy the full causal/informational chain above, including noise and internal use.

## Approved S3 principle: primitive physical coupling mechanisms, not senses

Observable classes and physical coupling mechanisms are many-to-many.

Mature mechanism families include, conceptually:

- binding/association/adsorption,
- chemical transformation/redox/protonation/reaction,
- state/phase/conformational transitions,
- mechanical force/torque/stress/strain/displacement transfer,
- charge redistribution/transport/polarization,
- radiative absorption/excitation,
- magnetic/spin coupling,
- ionization/energetic-particle deposition.

These are mechanism families, not an exhaustive hard-coded biological catalog.

Complex sensing can be a chain of PCPs.

Examples:

- humidity: water thermodynamics -> sorption/swelling -> mechanics -> transduction,
- gravity orientation: gravity/inertia -> displacement/loading -> mechanics,
- infrared: radiation -> heating -> thermal coupling OR radiation -> photochemistry,
- magnetic field: particle torque -> mechanics OR spin chemistry OR induction -> electrical coupling,
- ionizing radiation: ionization/radiolysis/secondary photons -> chemistry/optics/etc.

Geometry is a physical information transformer and may provide:

- filtering,
- resonance,
- shielding,
- focusing,
- apertures,
- directional selectivity,
- spatial sampling,
- inertial transformation,
- arrays.

Polymodal structures are allowed by default. One material/structure may participate in multiple PCPs.

Known Earth receptors/materials are validation examples, not the allowed search space.
Engineered transducers may be explored only as grounded extrapolations unless biologically demonstrated.

Unresolved mechanisms must coexist as alternative versioned models rather than being resolved by software convenience.

---

## Approved evidence/provenance categories

The docs should formalize evidence categories close to:

- **P1 — established physical interaction**
- **B1 — demonstrated biological sensory use**
- **B2 — demonstrated biological response, but sensory function/transduction incomplete**
- **M — mechanism unresolved / credible competing mechanisms**
- **X — mechanistically grounded extrapolation**
- **S — speculative**

Do not collapse these into one confidence percentage.

A mechanism or causal chain may contain several evidence statuses at different layers.

---

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

Issue #21 is the authority for the approved review conclusions. Its text does not supply a bibliographic list; the references below document supporting literature by topic, without claiming to reproduce an unavailable review bibliography or making further architecture choices. Earth mechanisms are validation examples. Evidence labels apply to individual physical, biological, and inferential layers; these references do not certify an alien mechanism. A concrete implementation still needs a versioned mechanism review, validity domain, parameters/units, alternatives, uncertainty, and benchmark evidence.

| Topic / literature | Evidence scope and limitation |
| --- | --- |
| [Evolution of Sensory Receptors (2024)](https://doi.org/10.1146/annurev-cellbio-120123-112853) | Biological receptor diversification, duplication/loss, mechanoreceptors, thermoreceptors, chemoreceptors and light receptors; B1 examples, not a universal catalog. |
| [Phototransduction and the Evolution of Photoreceptors (2010)](https://pmc.ncbi.nlm.nih.gov/articles/PMC2898276/) | Biological phototransduction and evolutionary diversity; B1 does not mandate eyes or opsins. |
| [The evolution and development of vertebrate lateral line electroreceptors (2013)](https://pmc.ncbi.nlm.nih.gov/articles/PMC4988487/) | B1 electroreception within a terrestrial lineage; not universal anatomy. |
| [Magnetosensation: the unsolved mystery (2026)](https://doi.org/10.1152/physrev.00032.2025) | Biological responses and competing transduction explanations must be separated (B2/M where incomplete). No universal magnetic mechanism is selected. |
| Berg and Purcell, [Physics of chemoreception (1977)](https://dash.harvard.edu/entities/publication/73120378-cafa-6bd4-e053-0100007fdf3b) | Physical sensing limits under declared assumptions (P1); not a generic sensor formula for every PCP. |
| Lan et al., [The energy–speed–accuracy trade-off in sensory adaptation (2012)](https://doi.org/10.1038/nphys2276) | Nonequilibrium tradeoffs in specified adaptive models and biological measurements; not an arbitrary universal cost. |

See [physical-coupling provenance](PHYSICAL_COUPLING_ARCHITECTURE.md#literature-and-provenance-notes) and [heritable-construction provenance](HERITABLE_CONSTRUCTION_ARCHITECTURE.md#literature-and-provenance-notes) for numerical and evolutionary context.
