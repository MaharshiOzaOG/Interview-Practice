// arb_item.sv : one arbitration cycle (req in, one-hot grant out)
class arb_item #(int N = 4) extends uvm_sequence_item;
  `uvm_object_param_utils(arb_item #(N))

  rand bit [N-1:0] req;
  bit      [N-1:0] gnt;

  function new(string name = "arb_item");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    arb_item #(N) rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "do_copy: wrong type")
    super.do_copy(rhs);
    req = rhs_c.req;
    gnt = rhs_c.gnt;
  endfunction

  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    arb_item #(N) rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    return (gnt == rhs_c.gnt);
  endfunction

  function string convert2string();
    return $sformatf("req=%b gnt=%b", req, gnt);
  endfunction
endclass
