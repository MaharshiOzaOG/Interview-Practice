// pipe_item.sv : data + time stamps (filled by monitor from vif counters)
class pipe_item extends uvm_sequence_item;
  `uvm_object_utils(pipe_item)

  rand bit [31:0] data;
  longint         cycle;   // in: accept cycle   / out: first out_valid cycle
  longint         stall;   // vif.stall_cnt at that moment

  function new(string name = "pipe_item");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    pipe_item rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "pipe_item::do_copy wrong type")
    super.do_copy(rhs);
    data  = rhs_c.data;
    cycle = rhs_c.cycle;
    stall = rhs_c.stall;
  endfunction

  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    pipe_item rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    return (data == rhs_c.data);           // timing checked separately
  endfunction

  function string convert2string();
    return $sformatf("data=%0h cyc=%0d stall=%0d", data, cycle, stall);
  endfunction
endclass

// expected entry = predicted item + when it entered
class pipe_exp;
  pipe_item item;
  longint   in_cycle;
  longint   in_stall;
  bit       timeout_reported;
endclass
