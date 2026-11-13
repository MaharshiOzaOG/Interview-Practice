// csr_item.sv : one register bus access
class csr_item extends uvm_sequence_item;
  `uvm_object_utils(csr_item)

  rand bit        write;      // 1 = write, 0 = read
  rand bit [7:0]  addr;
  rand bit [31:0] wdata;
  bit      [31:0] rdata;      // valid for reads
  bit             slverr;     // access to unmapped address

  function new(string name = "csr_item");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    csr_item rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "do_copy: wrong type")
    super.do_copy(rhs);
    write  = rhs_c.write;
    addr   = rhs_c.addr;
    wdata  = rhs_c.wdata;
    rdata  = rhs_c.rdata;
    slverr = rhs_c.slverr;
  endfunction

  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    csr_item rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    if (slverr != rhs_c.slverr) return 0;
    if (!write && (rdata != rhs_c.rdata)) return 0;
    return 1;
  endfunction

  function string convert2string();
    return $sformatf("%s addr=%0h wdata=%0h rdata=%0h slverr=%0b",
                     write ? "WR" : "RD", addr, wdata, rdata, slverr);
  endfunction
endclass
