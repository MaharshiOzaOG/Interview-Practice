// fifo_scoreboard.sv : one monitor gives one item per edge -> predict & compare immediately
//   (no queue needed: stimulus and response are in the same item)
class fifo_scoreboard #(int WIDTH = 8, int DEPTH = 8) extends uvm_scoreboard;
  `uvm_component_param_utils(fifo_scoreboard #(WIDTH, DEPTH))

  uvm_analysis_imp #(fifo_item #(WIDTH), fifo_scoreboard #(WIDTH, DEPTH)) mon_imp;

  fifo_ref_model #(WIDTH, DEPTH) ref_m;
  int pass_cnt;
  int fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    mon_imp = new("mon_imp", this);
    ref_m   = fifo_ref_model #(WIDTH, DEPTH)::type_id::create("ref_m");
  endfunction

  function void write(fifo_item #(WIDTH) t);
    fifo_item #(WIDTH) exp;
    exp = ref_m.predict(t);
    if (exp.compare(t)) pass_cnt++;
    else begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("MISMATCH\n exp: %s\n act: %s",
                                 exp.convert2string(), t.convert2string()))
    end
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("PASS=%0d FAIL=%0d", pass_cnt, fail_cnt), UVM_NONE)
  endfunction
endclass
