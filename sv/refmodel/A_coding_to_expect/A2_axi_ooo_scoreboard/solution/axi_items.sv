// axi_items.sv : AR request, R beat, memory write
typedef enum bit [1:0] {FIXED = 2'b00, INCR = 2'b01, WRAP = 2'b10} burst_e;

class axi_ar_item extends uvm_sequence_item;
  `uvm_object_utils(axi_ar_item)

  rand bit [3:0]  id;
  rand bit [31:0] addr;
  rand bit [7:0]  len;        // beats - 1
  rand burst_e    burst;

  constraint c_legal {
    addr[1:0] == 0;
    burst != 2'b11;
    (burst == WRAP) -> len inside {1, 3, 7, 15};
    len <= 15;
  }

  function new(string name = "axi_ar_item");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    axi_ar_item rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "axi_ar_item::do_copy wrong type")
    super.do_copy(rhs);
    id    = rhs_c.id;
    addr  = rhs_c.addr;
    len   = rhs_c.len;
    burst = rhs_c.burst;
  endfunction

  function string convert2string();
    return $sformatf("AR id=%0h addr=%0h len=%0d burst=%s", id, addr, len, burst.name());
  endfunction
endclass


class axi_r_item extends uvm_sequence_item;
  `uvm_object_utils(axi_r_item)

  bit [3:0]  id;
  bit [31:0] data;
  bit [1:0]  resp;
  bit        last;
  bit [31:0] addr;    // filled by model only, for messages

  function new(string name = "axi_r_item");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    axi_r_item rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "axi_r_item::do_copy wrong type")
    super.do_copy(rhs);
    id   = rhs_c.id;
    data = rhs_c.data;
    resp = rhs_c.resp;
    last = rhs_c.last;
    addr = rhs_c.addr;
  endfunction

  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    axi_r_item rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    return (id == rhs_c.id) && (data == rhs_c.data) &&
           (resp == rhs_c.resp) && (last == rhs_c.last);
  endfunction

  function string convert2string();
    return $sformatf("R id=%0h data=%0h resp=%0d last=%0b (addr=%0h)", id, data, resp, last, addr);
  endfunction
endclass


class mem_wr_item extends uvm_sequence_item;
  `uvm_object_utils(mem_wr_item)
  bit [31:0] addr;
  bit [31:0] data;
  function new(string name = "mem_wr_item");
    super.new(name);
  endfunction
endclass
