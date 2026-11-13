# A1 — Polymorphic predictor with factory swap (1-in → N-out)

**Bullet covered:** SV class with `predict(txn)`, queues, OOP.
**Concepts you must recall:** virtual class, pure virtual, polymorphism through a base handle, factory override, deep vs shallow copy (nested object + dynamic array), handwritten `do_copy` / `do_compare` / `convert2string` with inheritance (`super.` chain), `static` members, `protected`/`local`, `ref` arguments, `typedef class`.

## Statement

A **packet engine** DUT takes one input packet and produces one or more output packets. It has 3 modes, set by a config register:

| Mode | Output |
|---|---|
| `PASS` | the same packet |
| `CSUM` | same packet plus one extra payload byte = 8-bit sum of all payload bytes (wrap) |
| `SPLIT` | payload cut into chunks of `MAX_CHUNK` bytes, one packet per chunk; each has a **copy** of the header with `len` = chunk size and `seq` = 0,1,2… |

Packet classes:
- `pe_hdr` (a `uvm_object`): `src`, `dst` (8 bit), `len` (16 bit), `seq` (8 bit)
- `pe_pkt` (a `uvm_sequence_item`): `pe_hdr hdr` handle + `bit [7:0] payload[]`

Write:
1. `pe_hdr` and `pe_pkt` with **handwritten** `do_copy`, `do_compare`, `convert2string`. `pe_pkt::do_copy` must be a **deep** copy.
2. An abstract `pe_ref_base` with
   `pure virtual function void predict(pe_pkt in, ref pe_pkt out_q[$]);`
   and a `static int total_predicted` shared by every model instance.
3. Three children: `pe_ref_pass`, `pe_ref_csum`, `pe_ref_split`.
4. A scoreboard that holds only a **`pe_ref_base` handle**, creates it with `pe_ref_base::type_id::create`, and does in-order checking of all N outputs.
5. Show how a test picks the mode **only through a factory override**, without touching the scoreboard.

## Questions to answer after coding
1. What breaks in SPLIT mode if `do_copy` copies only the `hdr` handle?
2. Can you call `pe_ref_base::type_id::create` if `pe_ref_base` is `virtual`? What must exist for the override to work?
3. Why must `predict` take `out_q` as `ref` (or as an `output`)? What happens with a plain input argument?
4. Where does `total_predicted` live in memory, and what does it count when two scoreboards exist?

<details><summary><b>Hints</b> (open only if stuck)</summary>

- Deep copy = `new` the child object, then copy *its* fields. For dynamic arrays: `payload = new[rhs.payload.size()]` and a for loop.
- `do_compare` on the packet should call `hdr.compare(rhs.hdr)` so the header's `do_compare` is reused.
- A `virtual class` can't be constructed, so the factory can't create it. Keep the handle type as `pe_ref_base`, but `create` a **concrete default** (`pe_ref_pass`), and have the other models extend that default.
- Override: `set_type_override_by_type(pe_ref_pass::get_type(), pe_ref_split::get_type())` in the test's `build_phase`, **before** the env is built.
</details>

Solution: [`solution/`](solution/)
