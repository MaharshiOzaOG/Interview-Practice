// fifo_item.sv : what happened on ONE clock edge
//   Monitor samples at posedge: push/pop/wdata + full/empty BEFORE the edge,
//   and rdata for the pop.
class fifo_item #(int WIDTH = 8) extends uvm_sequence_item;
  `uvm_object_param_utils(fifo_item #(WIDTH))

  rand bit             push;
  rand bit             pop;
  rand bit [WIDTH-1:0] wdata;

  bit [WIDTH-1:0] rdata;
  bit             full;
  bit             empty;
  bit             overflow;    // push while full (and no pop)
  bit             underflow;   // pop while empty

  function new(string name = "fifo_item");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    fifo_item #(WIDTH) rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "do_copy: wrong type")
    super.do_copy(rhs);
    push      = rhs_c.push;
    pop       = rhs_c.pop;
    wdata     = rhs_c.wdata;
    rdata     = rhs_c.rdata;
    full      = rhs_c.full;
    empty     = rhs_c.empty;
    overflow  = rhs_c.overflow;
    underflow = rhs_c.underflow;
  endfunction

  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    fifo_item #(WIDTH) rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    // rdata only matters on a legal pop
    if (pop && !underflow && (rdata != rhs_c.rdata)) return 0;
    return (full == rhs_c.full) && (empty == rhs_c.empty) &&
           (overflow == rhs_c.overflow) && (underflow == rhs_c.underflow);
  endfunction

  function string convert2string();
    return $sformatf("push=%0b pop=%0b wdata=%0h | rdata=%0h full=%0b empty=%0b ovf=%0b udf=%0b",
                     push, pop, wdata, rdata, full, empty, overflow, underflow);
  endfunction
endclass
