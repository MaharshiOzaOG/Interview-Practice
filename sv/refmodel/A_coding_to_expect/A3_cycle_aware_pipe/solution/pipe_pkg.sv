// compile pipe_if.sv separately (interfaces can't go in a package)
package pipe_pkg;
  import uvm_pkg::*;
  `include "uvm_macros.svh"

  `include "pipe_item.sv"
  `include "pipe_sb.sv"
endpackage
