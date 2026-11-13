// arb_scoreboard.sv : policy chosen by config_db -> base handle points to the right child
class arb_scoreboard #(int N = 4) extends uvm_scoreboard;
  `uvm_component_param_utils(arb_scoreboard #(N))

  uvm_analysis_imp #(arb_item #(N), arb_scoreboard #(N)) mon_imp;

  arb_ref_base #(N) ref_m;     // base handle
  int pass_cnt;
  int fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    bit use_rr;
    super.build_phase(phase);
    mon_imp = new("mon_imp", this);
    if (!uvm_config_db #(bit)::get(this, "", "use_rr", use_rr)) use_rr = 1;
    if (use_rr) ref_m = arb_rr         #(N)::type_id::create("ref_m");
    else        ref_m = arb_fixed_prio #(N)::type_id::create("ref_m");
  endfunction

  function void write(arb_item #(N) t);
    arb_item #(N) exp;
    if (t.req == 0 && t.gnt == 0) return;       // idle cycle
    exp = ref_m.predict(t);
    if (exp.compare(t)) pass_cnt++;
    else begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("MISMATCH exp: %s act: %s",
                                 exp.convert2string(), t.convert2string()))
    end
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("PASS=%0d FAIL=%0d", pass_cnt, fail_cnt), UVM_NONE)
  endfunction
endclass
