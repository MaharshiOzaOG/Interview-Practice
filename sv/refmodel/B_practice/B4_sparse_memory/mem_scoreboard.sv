// mem_scoreboard.sv : monitor sends completed accesses in order
class mem_scoreboard extends uvm_scoreboard;
  `uvm_component_utils(mem_scoreboard)

  uvm_analysis_imp #(mem_item, mem_scoreboard) mon_imp;

  mem_ref_model ref_m;
  int pass_cnt;
  int fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    uninit_e pol;
    super.build_phase(phase);
    mon_imp = new("mon_imp", this);
    ref_m   = mem_ref_model::type_id::create("ref_m");
    if (uvm_config_db #(uninit_e)::get(this, "", "uninit_policy", pol))
      ref_m.policy = pol;
  endfunction

  function void write(mem_item t);
    mem_item exp;
    exp = ref_m.predict(t);
    if (exp.compare(t)) pass_cnt++;
    else begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("MISMATCH\n exp: %s\n act: %s",
                                 exp.convert2string(), t.convert2string()))
    end
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("PASS=%0d FAIL=%0d bytes_touched=%0d",
                              pass_cnt, fail_cnt, ref_m.mem.num()), UVM_NONE)
  endfunction
endclass
