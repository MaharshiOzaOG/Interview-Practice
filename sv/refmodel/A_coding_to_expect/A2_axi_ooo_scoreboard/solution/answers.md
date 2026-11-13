# A2 — Answers

1. **Aliasing:** `write()` passes a **handle**. If the monitor changes that same object at the next handshake, every stored handle changes with it. Clone where you **keep** something past the function call (`write_ar`). Each expected beat is also a fresh `create`. The coverage subscriber samples right away, so it needs no clone.
2. **WRAP len=3, addr=0x38:** total = 16, lower = 0x30 → `0x38, 0x3C, 0x30, 0x34`.
3. **Predict-time hazard:** data is predicted at AR time. If a write to the same address completes **after** the AR is sampled but **before** the slave reads its memory, the DUT may legally return the new data. Fixes: (a) the spec decides the ordering point, so predict at that point (e.g. at the first R beat), or (b) store both old and new value and accept either, or (c) keep the stimulus from writing to an address that has a read in flight.
4. **`uvm_subscriber`:** it is a `uvm_component` with a built-in `uvm_analysis_imp #(T, this_type) analysis_export` and `pure virtual write(T)`. One input type means no `_decl` macro is needed. Use `_decl` only when one component needs **several** `write` methods.

**Extra recall:** `foreach (exp_q[id])` walks only the keys that exist. `exp_q.delete(id)` removes one key. A covergroup declared inside a class must be `new`-ed in the class constructor.
