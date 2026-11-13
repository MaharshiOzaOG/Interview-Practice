// crc_dpi.sv : DPI declarations (inside the package)
import "DPI-C" function chandle      crc_new();
import "DPI-C" function void         crc_free  (chandle h);
import "DPI-C" function void         crc_update(chandle h, input byte unsigned data[]);
import "DPI-C" context function int unsigned crc_final(chandle h);  // calls sv_log -> context

export "DPI-C" function sv_log;

function void sv_log(string msg);
  `uvm_info("CRC_C", msg, UVM_HIGH)
endfunction
