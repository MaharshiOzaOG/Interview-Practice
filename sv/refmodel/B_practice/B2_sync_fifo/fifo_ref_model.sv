// fifo_ref_model.sv : hand-written circular buffer (no SV queue), same as the RTL idea
// Rules (spec):
//   flags are the state BEFORE the edge
//   push & pop when full  -> both happen (pop frees a slot)
//   push & pop when empty -> pop = underflow, push still happens
//   push when full (no pop) -> overflow, data dropped
class fifo_ref_model #(int WIDTH = 8, int DEPTH = 8) extends uvm_object;
  `uvm_object_param_utils(fifo_ref_model #(WIDTH, DEPTH))

  bit [WIDTH-1:0] mem [DEPTH];
  int             wr_ptr;
  int             rd_ptr;
  int             count;

  function new(string name = "fifo_ref_model");
    super.new(name);
    reset();
  endfunction

  function void reset();
    wr_ptr = 0;
    rd_ptr = 0;
    count  = 0;
  endfunction

  function fifo_item #(WIDTH) predict(fifo_item #(WIDTH) in);
    fifo_item #(WIDTH) exp;
    bit do_push;
    bit do_pop;

    exp = fifo_item #(WIDTH)::type_id::create("exp");
    exp.copy(in);

    // 1. flags from current state
    exp.full      = (count == DEPTH);
    exp.empty     = (count == 0);
    exp.underflow = in.pop  && exp.empty;
    exp.overflow  = in.push && exp.full && !in.pop;

    do_pop  = in.pop  && !exp.empty;
    do_push = in.push && (!exp.full || do_pop);

    // 2. read first (show-ahead data at rd_ptr)
    if (do_pop) begin
      exp.rdata = mem[rd_ptr];
      if (rd_ptr == DEPTH-1) rd_ptr = 0;
      else                   rd_ptr = rd_ptr + 1;
    end

    // 3. then write
    if (do_push) begin
      mem[wr_ptr] = in.wdata;
      if (wr_ptr == DEPTH-1) wr_ptr = 0;
      else                   wr_ptr = wr_ptr + 1;
    end

    count = count + do_push - do_pop;
    return exp;
  endfunction
endclass
