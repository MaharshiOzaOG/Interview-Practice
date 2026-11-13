// compile: crc_pkg.sv + crc_model.c (simulator compiles/links the C file)
package crc_pkg;
  import uvm_pkg::*;
  `include "uvm_macros.svh"

  `include "crc_dpi.sv"
  `include "crc_ref_models.sv"
  `include "crc_sb.sv"
endpackage
