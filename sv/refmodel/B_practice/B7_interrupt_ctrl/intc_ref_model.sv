// intc_ref_model.sv : cycle-stepped interrupt controller model
// Registers:
//   0x0 MASK     [N-1:0]  1 = enabled
//   0x4 CLEAR    [N-1:0]  W1C on pending (edge sources only)
//   0x8 EDGE_SEL [N-1:0]  1 = rising edge, 0 = level
//   0xC PRIO     [2N-1:0] 2-bit priority per source, higher wins; tie -> lower id
// Each edge, in this order:
//   1. bus write (config / clear)
//   2. pending: level src -> pending = src (live)
//               edge  src -> set on 0->1 (set wins over a clear in the same cycle)
//   3. irq = any(pending & mask); irq_id = best enabled pending source
class intc_ref_model #(int N = 8) extends uvm_object;
  `uvm_object_param_utils(intc_ref_model #(N))

  bit [N-1:0]   mask;
  bit [N-1:0]   edge_sel;
  bit [2*N-1:0] prio;
  bit [N-1:0]   pending;
  bit [N-1:0]   src_d;     // previous src, for edge detect

  function new(string name = "intc_ref_model");
    super.new(name);
    reset();
  endfunction

  function void reset();
    mask = 0; edge_sel = 0; prio = 0; pending = 0; src_d = 0;
  endfunction

  function int get_prio(int i);
    return prio[2*i +: 2];
  endfunction

  function intc_item #(N) step(intc_item #(N) in);
    intc_item #(N) exp;
    int best_id;
    int best_prio;

    exp = intc_item #(N)::type_id::create("exp");
    exp.copy(in);

    // 1. bus write
    if (in.wr_en) begin
      case (in.addr)
        4'h0: mask     = in.wdata[N-1:0];
        4'h4: pending  = pending & ~(in.wdata[N-1:0] & edge_sel);
        4'h8: edge_sel = in.wdata[N-1:0];
        4'hC: prio     = in.wdata[2*N-1:0];
      endcase
    end

    // 2. pending update
    for (int i = 0; i < N; i++) begin
      if (edge_sel[i]) begin
        if (in.src[i] && !src_d[i]) pending[i] = 1;
      end
      else pending[i] = in.src[i];
    end
    src_d = in.src;

    // 3. priority pick (manual loop, no built-ins)
    best_id   = -1;
    best_prio = -1;
    for (int i = 0; i < N; i++) begin
      if (pending[i] && mask[i] && (get_prio(i) > best_prio)) begin
        best_prio = get_prio(i);
        best_id   = i;
      end
    end

    exp.pending = pending;
    exp.irq     = (best_id >= 0);
    exp.irq_id  = (best_id >= 0) ? best_id : 0;
    return exp;
  endfunction
endclass
