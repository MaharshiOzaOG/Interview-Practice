// intc_item.sv : ONE clock edge: source lines, optional bus write, outputs after edge
class intc_item #(int N = 8) extends uvm_sequence_item;
  `uvm_object_param_utils(intc_item #(N))

  bit [N-1:0] src;
  bit         wr_en;
  bit [3:0]   addr;
  bit [31:0]  wdata;

  bit         irq;
  int         irq_id;     // valid when irq = 1
  bit [N-1:0] pending;    // DUT pending register (if visible)

  function new(string name = "intc_item");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    intc_item #(N) rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "do_copy: wrong type")
    super.do_copy(rhs);
    src     = rhs_c.src;
    wr_en   = rhs_c.wr_en;
    addr    = rhs_c.addr;
    wdata   = rhs_c.wdata;
    irq     = rhs_c.irq;
    irq_id  = rhs_c.irq_id;
    pending = rhs_c.pending;
  endfunction

  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    intc_item #(N) rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    if (irq != rhs_c.irq)                return 0;
    if (irq && (irq_id != rhs_c.irq_id)) return 0;
    return (pending == rhs_c.pending);
  endfunction

  function string convert2string();
    return $sformatf("src=%b wr=%0b addr=%0h wdata=%0h | irq=%0b id=%0d pend=%b",
                     src, wr_en, addr, wdata, irq, irq_id, pending);
  endfunction
endclass
