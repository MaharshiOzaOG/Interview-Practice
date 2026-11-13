// alu_fifo_sb.sv : FIFO-style in-order scoreboard (reference only)
//   needs alu_item / alu_ref_model from B1 (import alu_pkg::*)
class alu_fifo_sb extends uvm_scoreboard;
  `uvm_component_utils(alu_fifo_sb)

  uvm_tlm_analysis_fifo #(alu_item) in_fifo;
  uvm_tlm_analysis_fifo #(alu_item) out_fifo;

  alu_ref_model ref_m;
  int pass_cnt;
  int fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    in_fifo  = new("in_fifo",  this);
    out_fifo = new("out_fifo", this);
    ref_m    = alu_ref_model::type_id::create("ref_m");
  endfunction

  task run_phase(uvm_phase phase);
    alu_item in;
    alu_item act;
    alu_item exp;
    forever begin
      in_fifo.get(in);            // blocks until monitor wrote one
      exp = ref_m.predict(in);
      out_fifo.get(act);          // blocks until DUT produced one
      if (exp.compare(act)) pass_cnt++;
      else begin
        fail_cnt++;
        `uvm_error("SB", $sformatf("MISMATCH\n exp: %s\n act: %s",
                                   exp.convert2string(), act.convert2string()))
      end
    end
  endtask

  function void check_phase(uvm_phase phase);
    if (!in_fifo.is_empty())
      `uvm_error("SB", $sformatf("%0d inputs never checked", in_fifo.used()))
    if (!out_fifo.is_empty())
      `uvm_error("SB", $sformatf("%0d extra outputs", out_fifo.used()))
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("PASS=%0d FAIL=%0d", pass_cnt, fail_cnt), UVM_NONE)
  endfunction
endclass

// env connect_phase:
//   in_mon.ap.connect (sb.in_fifo.analysis_export);
//   out_mon.ap.connect(sb.out_fifo.analysis_export);
