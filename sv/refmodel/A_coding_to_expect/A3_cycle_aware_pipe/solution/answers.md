# A3 — Answers

1. **Stamp in the interface:** monitor `write()` calls and the scoreboard's `@(posedge clk)` thread all run at the same edge, and their order is not defined. A counter inside the scoreboard can be read before or after its increment, so latency is off by ±1 at random. Counters in the interface (updated with `<=`) give one value everyone reads the same way.
2. **Stall snapshot:** stall cycles are global, one output port. Saving `stall_cnt` at accept and taking the difference at output gives "stalls during *this* packet's life" in O(1), with no loop over all in-flight packets each cycle.
3. **Reset:**
   - **No flush:** the dropped packets stay at the head of the queue, so every output after reset is compared to stale data and fails, and `check_phase` complains.
   - **Flush, but the monitor still sends an `out` from the reset cycle:** you get an "output with nothing in flight" error. Fix it on the monitor side: don't publish while `!rst_n`.
4. **`phase_ready_to_end` vs `drain_time`:** a fixed drain time is a guess. Too short cuts real traffic, too long wastes time on every test. `phase_ready_to_end` waits exactly until the queue is empty and still has a timeout. Note that it can be called again after you drop the objection, which is why it checks `exp_q.size()` first.

**Extra recall:** a `fork ... join_none` inside a **function** is legal because it doesn't block. `void'(config_db::get(...))` = optional setting with a default. A plain (non-UVM) class works fine for internal bookkeeping (`pipe_exp`).
