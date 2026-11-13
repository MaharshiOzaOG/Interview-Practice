// axi_ar_cov.sv : uvm_subscriber = component + built-in analysis_export + write(T)
class axi_ar_cov extends uvm_subscriber #(axi_ar_item);
  `uvm_component_utils(axi_ar_cov)

  axi_ar_item tr;    // sampled item

  covergroup ar_cg;
    cp_burst : coverpoint tr.burst { bins b[] = {FIXED, INCR, WRAP}; }
    cp_len   : coverpoint tr.len   { bins single = {0};
                                     bins short_b = {[1:3]};
                                     bins long_b  = {[4:15]}; }
    cp_id    : coverpoint tr.id;
    x_burst_len : cross cp_burst, cp_len;
  endgroup

  function new(string name, uvm_component parent);
    super.new(name, parent);
    ar_cg = new();              // embedded covergroup: construct in new()
  endfunction

  function void write(axi_ar_item t);
    tr = t;                     // sample right away -> no clone needed here
    ar_cg.sample();
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("COV", $sformatf("AR coverage = %0.1f%%", ar_cg.get_coverage()), UVM_NONE)
  endfunction
endclass

// env connect_phase (for reference):
//   ar_mon.ap.connect(sb.ar_imp);
//   ar_mon.ap.connect(cov.analysis_export);
//   r_mon.ap.connect (sb.r_imp);
//   wr_mon.ap.connect(sb.wr_imp);
