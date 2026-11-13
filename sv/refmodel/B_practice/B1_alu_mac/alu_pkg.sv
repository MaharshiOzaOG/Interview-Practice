// alu_pkg.sv : compile this file; it pulls in the rest
package alu_pkg;
  import uvm_pkg::*;
  `include "uvm_macros.svh"

  `include "alu_item.sv"
  `include "alu_ref_model.sv"
  `include "alu_scoreboard.sv"
endpackage
