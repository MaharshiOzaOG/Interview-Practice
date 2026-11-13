// intc_scoreboard.sv : lock-step, one item per cycle
class intc_scoreboard #(int N = 8) extends uvm_scoreboard;
  `uvm_component_param_utils(intc_scoreboard #(N))

  uvm_analysis_imp #(intc_item #(N), intc_scoreboard #(N)) mon_imp;

  intc_ref_model #(N) ref_m;
  int pass_cnt;
  int fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    mon_imp = new("mon_imp", this);
    ref_m   = intc_ref_model #(N)::type_id::create("ref_m");
  endfunction

  function void write(intc_item #(N) t);
    intc_item #(N) exp;
    exp = ref_m.step(t);
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
