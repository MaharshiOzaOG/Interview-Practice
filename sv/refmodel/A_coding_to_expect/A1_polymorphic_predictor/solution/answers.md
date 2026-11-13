# A1 — Answers

1. **Shallow `hdr` copy in SPLIT:** every chunk would point to the *same* `pe_hdr` object. Setting `seq`/`len` on chunk 2 overwrites chunk 1, so all expected chunks end up with the last `seq`. The input item is also changed under the monitor. Fix: `create` a new header, then `copy`.
2. **Virtual base + factory:** no. A `virtual class` has no constructible `new`, so the factory can't build it. Register a **concrete** default (`pe_ref_pass`) and override it. The override type must **extend** the original type, which is why `csum`/`split` extend `pass`. The handle stays `pe_ref_base`, so calls stay polymorphic.
3. **`ref out_q`:** class handles pass by value, but a queue is a whole variable. With a plain `input` argument, `push_back` changes a **local copy** and the scoreboard queue stays empty. `ref` (or `output`) writes back to it. `ref` also avoids copying the queue.
4. **`static total_predicted`:** one variable per class *type*, not per object. It sits outside every object, so two scoreboards add to the **same** counter. Access it as `pe_ref_base::total_predicted`.

**Extra recall:** `exp.payload = new[n+1](exp.payload)` resizes and keeps the old content. `protected` lets children use `clone_pkt` but blocks outside code. Calling `super.do_copy` keeps the `uvm_sequence_item` fields (e.g. transaction id) correct.
