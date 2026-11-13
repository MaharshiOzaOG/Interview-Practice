# A4 — Golden C model through DPI-C, stateful, swappable with an SV model

**Bullet covered:** "a DPI-C hook to a C model".
**Concepts you must recall:** `import "DPI-C"` (`pure` vs `context`), `export "DPI-C"`, `chandle` for C-side state and its lifetime, open arrays (`input byte unsigned data[]` ↔ `svOpenArrayHandle`), SV↔C type mapping, who owns memory, polymorphism (the same base for the SV and C models), freeing in `final_phase`.

## Statement

DUT: a **streaming CRC-32 engine** (reflected, poly `0xEDB88320`, init `0xFFFFFFFF`, final XOR `0xFFFFFFFF`). It works per **channel**: software sends chunks of a message, and after the `last` chunk the DUT outputs the CRC. Up to 4 channels can be in progress at once.

The architecture team gives you a C golden model. Write:
1. **C side** (`crc_model.c`):
   - `void* crc_new(void)` / `void crc_free(void* h)`: one context per channel
   - `void crc_update(void* h, const svOpenArrayHandle data)`: process a chunk of any length
   - `unsigned int crc_final(void* h)`: return the CRC and reset the context
   - Call an **exported** SV function `sv_log(const char*)` on every `crc_final`.
2. **SV side:**
   - Imports with the right `pure`/`context` keywords.
   - Abstract `crc_ref_base` with `update(ch, data[])` and `final_crc(ch)`.
   - `crc_ref_sv`: the same algorithm in pure SV (bit loop, no built-ins).
   - `crc_ref_dpi`: holds `chandle h[4]`, creates them lazily, frees them in a `cleanup()` that the scoreboard calls from `final_phase`.
   - A scoreboard (`_chunk`, `_result` imps) that, in a debug mode, runs **both** models and errors if they disagree, before comparing with the DUT.

## Questions to answer after coding
1. Why must `crc_update` be `context` (or not)? Which import may be `pure`, and what does `pure` allow the simulator to do?
2. Who owns the memory behind a `chandle`? What happens if you never call `crc_free`? And if you call it twice?
3. How does C read an SV open array? Why not just pass `byte data[64]`?
4. Why run both models in debug mode? What does it catch that the DUT compare can't?

<details><summary><b>Hints</b></summary>

- `#include "svdpi.h"`, then `svSize(h, 1)` gives the element count and `svGetArrElemPtr1(h, i)` gives a pointer to element `i`.
- `int unsigned` ↔ `unsigned int`, `chandle` ↔ `void*`, `byte unsigned` ↔ `unsigned char`, `string` ↔ `const char*`.
- Only a `context` import may call an exported SV function.
- Bitwise CRC: for each byte `crc ^= b; repeat 8: crc = (crc & 1) ? (crc >> 1) ^ POLY : crc >> 1;`
- Check value: CRC-32 of ASCII `"123456789"` = `0xCBF43926`.
</details>

Solution: [`solution/`](solution/)
