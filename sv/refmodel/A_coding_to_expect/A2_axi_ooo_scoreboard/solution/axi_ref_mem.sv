// axi_ref_mem.sv : sparse word memory + burst address math
class axi_ref_mem extends uvm_object;
  `uvm_object_utils(axi_ref_mem)

  bit [31:0] mem[bit [31:0]];      // key = word-aligned byte address

  function new(string name = "axi_ref_mem");
    super.new(name);
  endfunction

  function void write_word(bit [31:0] addr, bit [31:0] data);
    mem[{addr[31:2], 2'b00}] = data;
  endfunction

  function bit [31:0] read_word(bit [31:0] addr);
    bit [31:0] a;
    a = {addr[31:2], 2'b00};
    if (mem.exists(a)) return mem[a];
    return 32'h0;
  endfunction

  // all beat addresses of one burst (size fixed at 4 bytes)
  function void calc_beats(axi_ar_item ar, ref bit [31:0] addr_q[$]);
    bit [31:0] a;
    bit [31:0] total;
    bit [31:0] lower;

    addr_q.delete();
    a     = ar.addr;
    total = (ar.len + 1) * 4;
    lower = (ar.addr / total) * total;

    for (int i = 0; i <= ar.len; i++) begin
      addr_q.push_back(a);
      case (ar.burst)
        FIXED: ;                                   // same address
        INCR : a = a + 4;
        WRAP : begin
          a = a + 4;
          if (a == lower + total) a = lower;
        end
      endcase
    end
  endfunction
endclass
