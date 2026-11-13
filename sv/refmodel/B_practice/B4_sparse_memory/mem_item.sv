// mem_item.sv : one 32-bit word access with byte strobes
class mem_item extends uvm_sequence_item;
  `uvm_object_utils(mem_item)

  rand bit        write;
  rand bit [31:0] addr;      // byte address, word aligned
  rand bit [31:0] wdata;
  rand bit [3:0]  strb;      // byte enables for writes
  bit      [31:0] rdata;

  constraint c_align { addr[1:0] == 0; }

  function new(string name = "mem_item");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    mem_item rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "do_copy: wrong type")
    super.do_copy(rhs);
    write = rhs_c.write;
    addr  = rhs_c.addr;
    wdata = rhs_c.wdata;
    strb  = rhs_c.strb;
    rdata = rhs_c.rdata;
  endfunction

  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    mem_item rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    if (write) return 1;                   // nothing observable on a write
    return (rdata === rhs_c.rdata);        // === so X policy can be checked
  endfunction

  function string convert2string();
    return $sformatf("%s addr=%0h wdata=%0h strb=%04b rdata=%0h",
                     write ? "WR" : "RD", addr, wdata, strb, rdata);
  endfunction
endclass
