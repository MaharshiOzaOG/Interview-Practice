// alu_item.sv : one ALU/MAC operation (stimulus + result)
class alu_item extends uvm_sequence_item;
  `uvm_object_utils(alu_item)

  typedef enum bit [2:0] {ADD, SUB, AND_OP, OR_OP, XOR_OP, MAC, CLR_ACC} op_e;

  // ---- stimulus ----
  rand op_e      op;
  rand bit [7:0] a;
  rand bit [7:0] b;

  // ---- response (filled by out monitor / ref model) ----
  bit [15:0] result;   // 8-bit ops use result[7:0]; MAC/CLR_ACC return the accumulator
  bit        c;        // carry (ADD) / no-borrow (SUB)
  bit        v;        // signed overflow (ADD/SUB)
  bit        z;        // result == 0
  bit        n;        // sign bit of result
  bit        sat;      // MAC saturated this cycle

  function new(string name = "alu_item");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    alu_item rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "do_copy: rhs is not alu_item")
    super.do_copy(rhs);
    op     = rhs_c.op;
    a      = rhs_c.a;
    b      = rhs_c.b;
    result = rhs_c.result;
    c      = rhs_c.c;
    v      = rhs_c.v;
    z      = rhs_c.z;
    n      = rhs_c.n;
    sat    = rhs_c.sat;
  endfunction

  // compare only the response fields: that is what the scoreboard checks
  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    alu_item rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    return (result == rhs_c.result) && (c == rhs_c.c) && (v == rhs_c.v) &&
           (z == rhs_c.z) && (n == rhs_c.n) && (sat == rhs_c.sat);
  endfunction

  function string convert2string();
    return $sformatf("op=%s a=%0h b=%0h | res=%0h c=%0b v=%0b z=%0b n=%0b sat=%0b",
                     op.name(), a, b, result, c, v, z, n, sat);
  endfunction
endclass
