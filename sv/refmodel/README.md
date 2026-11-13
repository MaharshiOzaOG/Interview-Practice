# Reference Model of DUT — SystemVerilog / UVM

Target: *"Developing Reference Model of DUT and Verification using HVL"* (Apple SoC DV).

**Style used everywhere:** snake_case with `_q` for queues, one class per file, a `*_pkg.sv` that `include`s the files, handwritten `do_copy` / `do_compare` / `convert2string` (no field macros), `uvm_analysis_imp_decl` + `write_*` (no analysis FIFO except A5), manual loops instead of array built-ins.

## Where things are

```
sv/refmodel/
├── A_coding_to_expect/          hard, concept-heavy problems
│   └── Ax_<name>/
│       ├── problem.md           statement + questions + hints (collapsed)
│       └── solution/            *.sv + answers.md
└── B_practice/                  sample code to read, then re-write
    └── Bx_<name>/
        ├── <x>_item.sv          transaction
        ├── <x>_ref_model.sv     the predictor
        ├── <x>_scoreboard.sv    wiring + compare
        └── <x>_pkg.sv           compile this
```

## A — Coding to expect (statement → hints → solution)

| # | Problem | Main concepts | Level |
|---|---|---|---|
| A1 | [Polymorphic predictor, factory swap, 1-in → N-out](A_coding_to_expect/A1_polymorphic_predictor/problem.md) | virtual class, pure virtual, factory override, deep copy, static, `ref` args | Hard |
| A2 | [Out-of-order AXI read scoreboard + coverage subscriber](A_coding_to_expect/A2_axi_ooo_scoreboard/problem.md) | 3× `imp_decl`, assoc array of queues, clone/aliasing, burst math, `uvm_subscriber` | Hard |
| A3 | [Cycle-aware pipe: latency, back-pressure, credits, reset](A_coding_to_expect/A3_cycle_aware_pipe/problem.md) | vif, race-free stamping, watchdog, reset flush, `phase_ready_to_end` | Hard |
| A4 | [Golden C model via DPI-C, swappable with SV model](A_coding_to_expect/A4_dpi_c_model/problem.md) | `context`/`pure`, `chandle`, open arrays, export, `final_phase` | Hard |
| A5 | [Same scoreboard with `uvm_tlm_analysis_fifo` (reference only)](A_coding_to_expect/A5_analysis_fifo_variant/problem.md) | blocking `get`, FIFO vs imp cost | Medium |

## B — Practice (sample code)

| # | Block | Model type | Key idea |
|---|---|---|---|
| B1 | [ALU / MAC](B_practice/B1_alu_mac/) | untimed, stateful acc | flags, signed saturation |
| B2 | [Sync FIFO](B_practice/B2_sync_fifo/) | per-edge, param class | hand-written circular buffer, ovf/udf |
| B3 | [CSR block](B_practice/B3_csr_block/) | RAL-style mirror | RW/RO/WO/W1C/RC policies, HW set |
| B4 | [Sparse memory](B_practice/B4_sparse_memory/) | assoc array | byte strobes, uninit-read policy |
| B5 | [Arbiter](B_practice/B5_arbiter/) | polymorphic policy | fixed priority vs round robin |
| B6 | [Timer](B_practice/B6_timer/) | cycle-stepped | prescaler, auto-reload, IRQ cycle |
| B7 | [Interrupt controller](B_practice/B7_interrupt_ctrl/) | cycle-stepped | mask, priority, edge vs level, W1C |

**How to practice:** for B, read the `ref_model`, close it, rewrite it from the spec comment at its top. For A, solve from `problem.md`, then answer the questions before you open `solution/`.
