# A3 — Cycle-aware predictor: latency window, back-pressure, credits, reset in the middle of traffic

**Bullet covered:** "some cycle awareness" (latency, ordering, back-pressure) with the model itself untimed.
**Concepts you must recall:** virtual interface via `uvm_config_db`, race-free time stamping (stamp in the interface, not the scoreboard), `run_phase` watchdog with `fork`/`join_none`, reset handling and flushing, `phase_ready_to_end` + objections for drain, a wrapper class for an expected item plus metadata, `local` constants and `int unsigned` math.

## Statement

DUT: a **credit-limited in-order pipe**.
- Input `in_valid/in_ready/in_data[31:0]`, output `out_valid/out_ready/out_data[31:0]`.
- Transform: `out_data = {in_data[15:0], in_data[31:16]} ^ KEY`. `KEY` is a constant you get from config.
- Order is kept.
- **Latency** = cycles from input accept to the first cycle `out_valid` is high, **minus** cycles the output was stalled (`out_valid && !out_ready`) in that window. It must be in `[MIN_LAT, MAX_LAT]`.
- At most `CREDITS` packets in flight. Accepting more is a DUT bug.
- `rst_n` low in the middle of traffic: every packet in flight is **dropped** (no output expected).

Write:
1. The interface counters: `cycle_cnt` and `stall_cnt`, updated in the interface. Monitors stamp items with both.
2. `pipe_sb` with `_in` / `_out` imps that checks data, order, the latency window and credits.
3. A `run_phase` watchdog that flags a packet stuck longer than `MAX_LAT + stalls` (a lost packet), and **flushes** the expected queue on reset.
4. `phase_ready_to_end`: if packets are still in flight when the test wants to end, hold the phase until they drain or a timeout hits.

## Questions to answer after coding
1. Why stamp time in the interface and not with a cycle counter inside the scoreboard's `run_phase`?
2. Why is `stall_cnt` saved as a **snapshot** at accept time, and not counted per packet?
3. What goes wrong if the scoreboard doesn't flush on reset, and if it flushes but the monitor still sends an `out` item from the reset cycle?
4. `phase_ready_to_end` vs a fixed `drain_time`: which is better and why?

<details><summary><b>Hints</b></summary>

- Expected wrapper: `class pipe_exp; pipe_item item; longint in_cycle; longint in_stall; endclass`.
- Latency at output = `(out.cycle - e.in_cycle) - (out.stall - e.in_stall)`.
- Watchdog: `forever @(posedge vif.clk)`. Check `exp_q[0]`: `vif.cycle_cnt - in_cycle - (vif.stall_cnt - in_stall) > MAX_LAT`.
- Reset: in the watchdog, `if (!vif.rst_n) exp_q.delete();`. Monitors should not send items while `rst_n` is low.
- `phase_ready_to_end(uvm_phase phase)`: if the phase is `run` and `exp_q.size() > 0`, then `phase.raise_objection(this)` and `fork` a thread that waits, then drops the objection.
</details>

Solution: [`solution/`](solution/)
