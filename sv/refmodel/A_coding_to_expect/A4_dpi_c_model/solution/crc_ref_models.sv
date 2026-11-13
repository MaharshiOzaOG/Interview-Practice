// crc_ref_models.sv : same contract, two implementations
virtual class crc_ref_base extends uvm_object;
  function new(string name = "crc_ref_base");
    super.new(name);
  endfunction
  pure virtual function void         update(int ch, byte unsigned data[]);
  pure virtual function int unsigned final_crc(int ch);
  virtual function void cleanup();
  endfunction
endclass


// ---- pure SV model ----
class crc_ref_sv extends crc_ref_base;
  `uvm_object_utils(crc_ref_sv)

  localparam bit [31:0] POLY = 32'hEDB88320;
  bit [31:0] crc[4];

  function new(string name = "crc_ref_sv");
    super.new(name);
    for (int i = 0; i < 4; i++) crc[i] = 32'hFFFFFFFF;
  endfunction

  virtual function void update(int ch, byte unsigned data[]);
    for (int i = 0; i < data.size(); i++) begin
      crc[ch] = crc[ch] ^ data[i];
      for (int k = 0; k < 8; k++) begin
        if (crc[ch][0]) crc[ch] = (crc[ch] >> 1) ^ POLY;
        else            crc[ch] = (crc[ch] >> 1);
      end
    end
  endfunction

  virtual function int unsigned final_crc(int ch);
    int unsigned r;
    r       = crc[ch] ^ 32'hFFFFFFFF;
    crc[ch] = 32'hFFFFFFFF;
    return r;
  endfunction
endclass


// ---- C golden model through DPI ----
class crc_ref_dpi extends crc_ref_base;
  `uvm_object_utils(crc_ref_dpi)

  chandle h[4];      // null until first use

  function new(string name = "crc_ref_dpi");
    super.new(name);
  endfunction

  local function void ensure(int ch);
    if (h[ch] == null) h[ch] = crc_new();
  endfunction

  virtual function void update(int ch, byte unsigned data[]);
    ensure(ch);
    crc_update(h[ch], data);
  endfunction

  virtual function int unsigned final_crc(int ch);
    ensure(ch);
    return crc_final(h[ch]);
  endfunction

  virtual function void cleanup();
    for (int i = 0; i < 4; i++) begin
      if (h[i] != null) begin
        crc_free(h[i]);
        h[i] = null;           // avoid double free
      end
    end
  endfunction
endclass
