// csr_model.sv : mini RAL-style mirror, written by hand
//   csr_field : one field + its access policy
//   csr_reg   : list of fields at one address
//   csr_model : map addr -> reg, predicts bus reads/writes, takes HW events
typedef enum {RW, RO, WO, W1C, RC} access_e;

class csr_field extends uvm_object;
  `uvm_object_utils(csr_field)

  string     fname;
  int        lsb;
  int        width;
  access_e   access;
  bit [31:0] reset_val;
  bit [31:0] value;        // mirrored value (right-aligned)

  function new(string name = "csr_field");
    super.new(name);
  endfunction

  function void configure(string fname, int lsb, int width, access_e access, bit [31:0] reset_val);
    this.fname     = fname;
    this.lsb       = lsb;
    this.width     = width;
    this.access    = access;
    this.reset_val = reset_val;
    this.value     = reset_val;
  endfunction

  function bit [31:0] mask();
    return ((32'h1 << width) - 1);
  endfunction

  // bus write: wbits already shifted down to bit 0
  function void bus_write(bit [31:0] wbits);
    wbits = wbits & mask();
    case (access)
      RW, WO : value = wbits;
      W1C    : value = value & ~wbits;     // write 1 clears
      RO, RC : ;                           // bus write ignored
    endcase
  endfunction

  // bus read: returns what the bus sees, applies side effect
  function bit [31:0] bus_read();
    bit [31:0] rd;
    case (access)
      WO     : rd = 0;
      default: rd = value;
    endcase
    if (access == RC) value = 0;           // read clears
    return rd;
  endfunction
endclass


class csr_reg extends uvm_object;
  `uvm_object_utils(csr_reg)

  string    rname;
  csr_field fields[$];

  function new(string name = "csr_reg");
    super.new(name);
  endfunction

  function void add_field(string fname, int lsb, int width, access_e access, bit [31:0] reset_val);
    csr_field f;
    f = csr_field::type_id::create(fname);
    f.configure(fname, lsb, width, access, reset_val);
    fields.push_back(f);
  endfunction

  function void write(bit [31:0] wdata);
    for (int i = 0; i < fields.size(); i++)
      fields[i].bus_write(wdata >> fields[i].lsb);
  endfunction

  function bit [31:0] read();
    bit [31:0] rd;
    rd = 0;
    for (int i = 0; i < fields.size(); i++)
      rd = rd | (fields[i].bus_read() << fields[i].lsb);
    return rd;
  endfunction

  function void reset();
    for (int i = 0; i < fields.size(); i++)
      fields[i].value = fields[i].reset_val;
  endfunction

  function csr_field get_field(string fname);
    for (int i = 0; i < fields.size(); i++)
      if (fields[i].fname == fname) return fields[i];
    return null;
  endfunction
endclass


class csr_model extends uvm_object;
  `uvm_object_utils(csr_model)

  csr_reg regs[bit [7:0]];

  function new(string name = "csr_model");
    super.new(name);
  endfunction

  // example map: CTRL / STATUS / INT_STAT / DATA
  function void build();
    csr_reg r;

    r = csr_reg::type_id::create("CTRL");
    r.add_field("EN",   0, 1, RW, 0);
    r.add_field("MODE", 1, 3, RW, 3'h2);
    r.add_field("KEY",  8, 8, WO, 0);
    regs[8'h00] = r;

    r = csr_reg::type_id::create("STATUS");
    r.add_field("BUSY", 0, 1, RO, 0);
    r.add_field("VER",  4, 4, RO, 4'h3);
    regs[8'h04] = r;

    r = csr_reg::type_id::create("INT_STAT");
    r.add_field("DONE", 0, 1, W1C, 0);
    r.add_field("ERR",  1, 1, W1C, 0);
    regs[8'h08] = r;

    r = csr_reg::type_id::create("ERR_CNT");
    r.add_field("CNT",  0, 8, RC, 0);
    regs[8'h0C] = r;
  endfunction

  function csr_item predict(csr_item in);
    csr_item exp;
    exp = csr_item::type_id::create("exp");
    exp.copy(in);
    exp.rdata  = 0;
    exp.slverr = 0;

    if (!regs.exists(in.addr)) begin
      exp.slverr = 1;
      return exp;
    end
    if (in.write) regs[in.addr].write(in.wdata);
    else          exp.rdata = regs[in.addr].read();
    return exp;
  endfunction

  // HW side events (from a DUT-internal monitor): set RO/W1C bits, bump counters
  function void hw_set(bit [7:0] addr, string fname, bit [31:0] val);
    csr_field f;
    if (!regs.exists(addr)) return;
    f = regs[addr].get_field(fname);
    if (f == null) return;
    if (f.access == W1C) f.value = f.value | (val & f.mask());   // sticky set
    else                 f.value = val & f.mask();
  endfunction

  function void reset();
    foreach (regs[a]) regs[a].reset();
  endfunction
endclass
