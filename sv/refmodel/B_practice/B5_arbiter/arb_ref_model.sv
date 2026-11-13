// arb_ref_model.sv : base predictor + two policies (polymorphism)
//   arb_ref_base   : common predict(), pure virtual pick()
//   arb_fixed_prio : lowest index wins
//   arb_rr         : search starts after last winner; pointer moves only on a grant
virtual class arb_ref_base #(int N = 4) extends uvm_object;

  function new(string name = "arb_ref_base");
    super.new(name);
  endfunction

  pure virtual function int pick(bit [N-1:0] req);   // returns index or -1
  pure virtual function void reset();

  function arb_item #(N) predict(arb_item #(N) in);
    arb_item #(N) exp;
    int w;
    exp = arb_item #(N)::type_id::create("exp");
    exp.copy(in);
    exp.gnt = 0;
    w = pick(in.req);
    if (w >= 0) exp.gnt[w] = 1'b1;
    return exp;
  endfunction
endclass


class arb_fixed_prio #(int N = 4) extends arb_ref_base #(N);
  `uvm_object_param_utils(arb_fixed_prio #(N))

  function new(string name = "arb_fixed_prio");
    super.new(name);
  endfunction

  virtual function int pick(bit [N-1:0] req);
    for (int i = 0; i < N; i++)
      if (req[i]) return i;
    return -1;
  endfunction

  virtual function void reset();
  endfunction
endclass


class arb_rr #(int N = 4) extends arb_ref_base #(N);
  `uvm_object_param_utils(arb_rr #(N))

  int last;    // last granted index

  function new(string name = "arb_rr");
    super.new(name);
    reset();
  endfunction

  virtual function void reset();
    last = N - 1;     // so requester 0 is first after reset
  endfunction

  virtual function int pick(bit [N-1:0] req);
    int idx;
    for (int k = 1; k <= N; k++) begin
      idx = (last + k) % N;
      if (req[idx]) begin
        last = idx;
        return idx;
      end
    end
    return -1;
  endfunction
endclass
