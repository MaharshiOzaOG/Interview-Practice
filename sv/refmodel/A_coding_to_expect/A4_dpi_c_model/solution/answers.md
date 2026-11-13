# A4 — Answers

1. **`context` / `pure`:**
   - `crc_final` calls the exported `sv_log`, so it **must** be `context`. A `context` import gets the caller's scope, which is what it needs to call back into SV.
   - `crc_update` doesn't call SV, so a plain import is enough.
   - None of these can be `pure`, because they change C state. `pure` promises no side effects and output based only on inputs, so the simulator may skip or cache the call. A stateless helper like `crc_byte(crc, b)` could be `pure`.
2. **`chandle` ownership:** C allocated the memory, so C must free it. SV only holds an opaque pointer and garbage collection never touches it.
   - Never calling `crc_free` leaks memory. It hurts in long regressions and emulation.
   - Calling it twice is a double free, which is undefined behavior and usually a crash. Hence `h[i] = null` after freeing, plus the null check.
3. **Open array:** C gets an `svOpenArrayHandle`, reads the length with `svSize(h, 1)` and gets elements with `svGetArrElemPtr1(h, i)`. A fixed `byte data[64]` forces one size and wastes or truncates data. The open array accepts any SV dynamic array.
4. **Two models:** if the C model is called wrongly (e.g. a type mapping bug, a signed `byte` instead of `byte unsigned`, a context not reset between messages), the DUT will "fail" and you will debug the RTL for nothing. Comparing two independent models separates **model or integration bugs** from DUT bugs.

**Extra recall:** DPI imports and exports may sit inside a package. Interfaces may not. `final_phase` runs once at the very end, which makes it the right place to release outside resources.
