// timer_item.sv : ONE clock edge. Bus write (if any) + irq seen after that edge.
class timer_item extends uvm_sequence_item;
  `uvm_object_utils(timer_item)

  bit        wr_en;
  bit [3:0]  addr;     // 0 = CTRL, 4 = LOAD
  bit [31:0] wdata;
  bit        irq;      // DUT output (1-cycle pulse)
  longint    cycle;    // stamped by monitor, for messages

  function new(string name = "timer_item");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    timer_item rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "do_copy: wrong type")
    super.do_copy(rhs);
    wr_en = rhs_c.wr_en;
    addr  = rhs_c.addr;
    wdata = rhs_c.wdata;
    irq   = rhs_c.irq;
    cycle = rhs_c.cycle;
  endfunction

  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    timer_item rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    return (irq == rhs_c.irq);
  endfunction

  function string convert2string();
    return $sformatf("cyc=%0d wr=%0b addr=%0h wdata=%0h irq=%0b", cycle, wr_en, addr, wdata, irq);
  endfunction
endclass
