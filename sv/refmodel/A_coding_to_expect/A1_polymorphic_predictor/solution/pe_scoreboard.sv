// pe_scoreboard.sv : knows only the BASE handle; the factory decides the real model
`uvm_analysis_imp_decl(_in)
`uvm_analysis_imp_decl(_out)

class pe_scoreboard extends uvm_scoreboard;
  `uvm_component_utils(pe_scoreboard)

  uvm_analysis_imp_in  #(pe_pkt, pe_scoreboard) in_imp;
  uvm_analysis_imp_out #(pe_pkt, pe_scoreboard) out_imp;

  pe_ref_base ref_m;      // base handle -> polymorphic predict()
  pe_pkt      exp_q[$];
  int         pass_cnt;
  int         fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    in_imp  = new("in_imp",  this);
    out_imp = new("out_imp", this);
    // create the DEFAULT concrete type; an override turns it into csum/split
    ref_m = pe_ref_pass::type_id::create("ref_m");
    `uvm_info("SB", $sformatf("Reference model in use: %s", ref_m.get_type_name()), UVM_LOW)
  endfunction

  function void write_in(pe_pkt t);
    ref_m.predict(t, exp_q);          // may push 1..N items
  endfunction

  function void write_out(pe_pkt t);
    pe_pkt exp;
    if (exp_q.size() == 0) begin
      `uvm_error("SB", $sformatf("Unexpected output %s", t.convert2string()))
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
      `uvm_error("SB", $sformatf("%0d expected packets never came out", exp_q.size()))
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("PASS=%0d FAIL=%0d total_predicted(all models)=%0d",
                              pass_cnt, fail_cnt, pe_ref_base::total_predicted), UVM_NONE)
  endfunction
endclass
