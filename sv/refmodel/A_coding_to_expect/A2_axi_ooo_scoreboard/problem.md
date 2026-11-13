# A2 — Out-of-order AXI read scoreboard with predictor + coverage subscriber

**Bullet covered:** wiring the model into UVM with `uvm_analysis_imp_decl` + `write_*`, `uvm_subscriber`, in-order inside an ID and out-of-order across IDs.
**Concepts you must recall:** multiple imps in one component, associative array of queues, handle aliasing (monitor reusing one object), `clone()` + `$cast`, AXI burst address math (FIXED / INCR / WRAP), `check_phase` leftovers, covergroup inside a class (`uvm_subscriber`), `uvm_config_db`, `foreach` over an associative array.

## Statement

DUT: an AXI4 **read-only memory slave**, 32-bit data, `ARSIZE` = 4 bytes. It can return R data **out of order across IDs** but must keep order **within** one ID. Its memory is also written by a separate path, and you get those writes from a `mem_wr` monitor (addr, data).

Monitors you have:
- `ar_mon` → `axi_ar_item` (id, addr, len, burst) at the AR handshake
- `r_mon` → `axi_r_item` (id, data, resp, last) for every R beat
- `wr_mon` → `mem_wr_item` (addr, data) for every word written

Write:
1. `axi_ref_mem`: sparse word memory + `function void calc_beats(axi_ar_item ar, ref bit [31:0] addr_q[$])` that does FIXED/INCR/WRAP address math.
2. `axi_ooo_sb` with **three** imps (`_ar`, `_r`, `_wr`) and `exp_q[bit [3:0]][$]`, an associative array of per-ID queues of expected beats.
   - On AR: compute all beats, push them to `exp_q[id]` with the right `last` flag.
   - On R: pop the head of `exp_q[id]` and compare data + `last` + `resp == OKAY`. An R beat for an ID with nothing pending is an error.
   - In `check_phase`: report every ID that still has beats left.
3. `axi_ar_cov` extending `uvm_subscriber #(axi_ar_item)`: cover burst × len buckets × ID, plus a cross of burst × len.
4. Assume the AR monitor **reuses one item object** for every handshake. Make your scoreboard safe against that.

## Questions to answer after coding
1. Why is storing the AR handle unsafe if the monitor reuses it? Where exactly do you need `clone`?
2. WRAP with len=3 (4 beats), addr=0x38: list the 4 addresses.
3. When could the predicted data be wrong even though the DUT is right? (Think: a write landing between AR and R.) How would you fix that?
4. Why does `uvm_subscriber` give you `analysis_export` and not an imp you declare?

<details><summary><b>Hints</b></summary>

- `axi_r_item exp_q[bit [3:0]][$];` then `exp_q[id].push_back(x)`. Check `exp_q.exists(id) && exp_q[id].size() > 0` before popping.
- WRAP: `total = (len+1)*4`, `lower = (addr / total) * total`, next addr = `addr + 4`, and if it reaches `lower + total` set it back to `lower`.
- FIXED: the same address every beat.
- Predict data **at AR time** = what memory holds when the slave samples the request. Question 3 is about that choice.
- `uvm_subscriber` already has `analysis_export` and a pure virtual `write(T t)` you must implement.
</details>

Solution: [`solution/`](solution/)
