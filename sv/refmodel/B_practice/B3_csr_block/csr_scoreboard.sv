// csr_scoreboard.sv : bus monitor gives completed accesses; hw monitor gives HW events
`uvm_analysis_imp_decl(_bus)
`uvm_analysis_imp_decl(_hw)

class csr_hw_event extends uvm_object;
  `uvm_object_utils(csr_hw_event)
  bit [7:0]  addr;
  string     fname;
  bit [31:0] val;
  function new(string name = "csr_hw_event");
    super.new(name);
  endfunction
endclass

class csr_scoreboard extends uvm_scoreboard;
  `uvm_component_utils(csr_scoreboard)

  uvm_analysis_imp_bus #(csr_item,     csr_scoreboard) bus_imp;
  uvm_analysis_imp_hw  #(csr_hw_event, csr_scoreboard) hw_imp;

  csr_model model;
  int pass_cnt;
  int fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    bus_imp = new("bus_imp", this);
    hw_imp  = new("hw_imp",  this);
    model   = csr_model::type_id::create("model");
    model.build();
  endfunction

  function void write_bus(csr_item t);
    csr_item exp;
    exp = model.predict(t);
    if (exp.compare(t)) pass_cnt++;
    else begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("MISMATCH\n exp: %s\n act: %s",
                                 exp.convert2string(), t.convert2string()))
    end
  endfunction

  function void write_hw(csr_hw_event e);
    model.hw_set(e.addr, e.fname, e.val);
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("PASS=%0d FAIL=%0d", pass_cnt, fail_cnt), UVM_NONE)
  endfunction
endclass
