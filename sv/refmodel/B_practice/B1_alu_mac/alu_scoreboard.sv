// alu_scoreboard.sv : in-order scoreboard, two imps (input side / output side)
`uvm_analysis_imp_decl(_in)
`uvm_analysis_imp_decl(_out)

class alu_scoreboard extends uvm_scoreboard;
  `uvm_component_utils(alu_scoreboard)

  uvm_analysis_imp_in  #(alu_item, alu_scoreboard) in_imp;
  uvm_analysis_imp_out #(alu_item, alu_scoreboard) out_imp;

  alu_ref_model ref_m;
  alu_item      exp_q[$];
  int           pass_cnt;
  int           fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    in_imp  = new("in_imp",  this);
    out_imp = new("out_imp", this);
    ref_m   = alu_ref_model::type_id::create("ref_m");
  endfunction

  // input monitor -> predict now, store expected
  function void write_in(alu_item t);
    exp_q.push_back(ref_m.predict(t));
  endfunction

  // output monitor -> compare with oldest expected
  function void write_out(alu_item t);
    alu_item exp;
    if (exp_q.size() == 0) begin
      `uvm_error("SB", $sformatf("Unexpected output: %s", t.convert2string()))
      return;
    end
    exp = exp_q.pop_front();
    if (exp.compare(t)) pass_cnt++;
    else begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("MISMATCH\n exp: %s\n act: %s",
                                 exp.convert2string(), t.convert2string()))
    end
  endfunction

  function void check_phase(uvm_phase phase);
    if (exp_q.size() != 0)
      `uvm_error("SB", $sformatf("%0d expected items never came out", exp_q.size()))
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("PASS=%0d FAIL=%0d", pass_cnt, fail_cnt), UVM_NONE)
  endfunction
endclass
