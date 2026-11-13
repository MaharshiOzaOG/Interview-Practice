# A5 — The same scoreboard with `uvm_tlm_analysis_fifo` (reference only)

**Why it's separate:** your main style uses `uvm_analysis_imp_decl` + `write_*`. This one shows the FIFO style so you can answer "how else would you do it?"

**Concepts you must recall:** `uvm_tlm_analysis_fifo #(T)` (an unbounded FIFO with an `analysis_export`), blocking `get()` in `run_phase`, `try_get()`, `is_empty()` / `used()`, connecting to `fifo.analysis_export`, and the time and memory cost compared with imps.

## Statement

Reuse the ALU/MAC items and reference model from [B1](../../B_practice/B1_alu_mac/). Write `alu_fifo_sb` that:
1. Has `in_fifo` and `out_fifo` (`uvm_tlm_analysis_fifo #(alu_item)`).
2. In `run_phase`: `forever` → `in_fifo.get(in)` → predict → `out_fifo.get(act)` → compare.
3. In `check_phase`: report anything left in either FIFO.
4. Show the env `connect_phase` lines.

## Questions
1. Compare imp vs FIFO: when does each one predict and compare, in what context (function vs task), and at what cost?
2. When is the FIFO style actually the better choice?
3. What happens if the monitor reuses its item object? Does the FIFO protect you?

<details><summary><b>Hints</b></summary>

- `get()` is a **task** and blocks until an item arrives, so it only works in `run_phase`.
- The `analysis_export` of the FIFO calls `try_put` (non-blocking) and stores the **handle**.
</details>

Solution: [`solution/alu_fifo_sb.sv`](solution/alu_fifo_sb.sv)
