# V2Fun — original assets for DELIVER CHAOS

Initial live balance on 2026-10-31: **5,100 API credits**. Latest verified available balance: **4,910**. Difference: **190** (168 for four completed model generations, 22 for one failed texture task; the failed task refund has not yet appeared in the queried balance). No other provider credits were consumed in this pass.

| Asset | Job | Credits | Result |
|---|---|---:|---|
| Dafu independent silver-tabby mesh | b2bd32ef-99e4-4ba4-adc4-2d252c1ea829 | 42 | 32,000 triangles; local quadruped skinning; imported with shared Ahuai animation skeleton |
| PetCamera | bdae14af-00c8-43c4-88d3-db47da0e9c3d | 42 | 7,000 triangles; imported shell, existing simple collision and camera gameplay retained |
| HalloweenCake V1 | c138c6fe-5a73-44c1-a6ec-fec67ab077d3 | 42 | Superseded by the cleaner V2 reference; archived outside the shipped project |
| HalloweenCake V2 | d467f45b-2736-4178-abf4-b848e9751810 | 42 | 7,000 triangles; imported as a destructible ¥329 centerpiece |
| Dafu shared-topology texture experiment | eaf49c65-648d-41d0-a498-c9768a102fde | 22 | Provider reported FAILED; no automatic retry, refund unconfirmed; not used |

The independent Dafu mesh became usable after fixing the processing pipeline: weld split vertices before decimation, weld again before Blender bone-heat skinning, normalize at most four influences, and validate five gesture poses plus walk/run. V2Fun did not supply a quadruped skeleton. Earlier nearest-surface weight transfer was rejected. Local processing uses the existing Hyper3D quadruped skeleton and animations, whose separate source terms remain applicable.

Production settings: pro geometry, standard texture, PBR; no 8K upcharge. Target runtime budgets: cat <=35k triangles / 2k textures, props <=8k / 1–2k textures, simple convex gameplay collision. All three retained assets were inspected in Blender from four angles and imported into Unreal; package-level checks are recorded separately in Verification/v2fun-build.json.

Reference: Dafu is an original imagegen variation of the approved Ahuai art direction. The original three cats are preserved. `../References/Generated/dafu-v1.png` is a new production reference, not a claim of further user approval.

## Attribution and rights

Generated using V2Fun, https://v2fun.art/ . Prompts and subsequent modeling, optimization, materials and integration by the DELIVER CHAOS project. Terms checked 2026-10-31: https://v2fun.art/agreement/terms-of-use §6.3 distinguishes paid output ownership from free output under CC BY 4.0. Account plan classification for gifted API credits has not been established; preserve this attribution and the conservative CC BY 4.0 notice (https://creativecommons.org/licenses/by/4.0/) with generated assets and shipped credits. Do not claim exclusive ownership solely because credits were available. Any third-party elements retain their own restrictions.

## Reproduction

- Official regional API: https://api.v2fun.art/api/v1
- Documentation and pricing: https://doc.v2fun.art/ and https://doc.v2fun.art/pricing
- Use `Tools/v2fun_assets.py` with `V2FUN_API_KEY` in the local environment or the explicit `--clipboard` option. Never put a key in the project, a command argument, a log, or GitHub.
- POST is never retried automatically. State is stored outside the repository in the workspace `work/v2fun` directory, including signed download links. `provenance.json` contains only durable identifiers and file hashes.
- Poll existing task IDs after network errors before deciding whether to submit anything else.
