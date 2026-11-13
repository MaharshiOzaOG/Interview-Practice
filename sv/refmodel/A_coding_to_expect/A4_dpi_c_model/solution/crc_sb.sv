// crc_sb.sv : chunk monitor feeds models, result monitor gives DUT CRC
class crc_chunk_item extends uvm_sequence_item;
  `uvm_object_utils(crc_chunk_item)
  int           ch;
  byte unsigned data[];
  bit           last;
  function new(string name = "crc_chunk_item");
    super.new(name);
  endfunction
endclass

class crc_result_item extends uvm_sequence_item;
  `uvm_object_utils(crc_result_item)
  int          ch;
  int unsigned crc;
  function new(string name = "crc_result_item");
    super.new(name);
  endfunction
endclass

`uvm_analysis_imp_decl(_chunk)
`uvm_analysis_imp_decl(_result)

class crc_sb extends uvm_scoreboard;
  `uvm_component_utils(crc_sb)

  uvm_analysis_imp_chunk  #(crc_chunk_item,  crc_sb) chunk_imp;
  uvm_analysis_imp_result #(crc_result_item, crc_sb) result_imp;

  crc_ref_base gold;        // DPI model (main)
  crc_ref_base shadow;      // SV model (debug cross-check)
  bit          debug;
  int unsigned exp_q[4][$]; // per channel expected CRCs
  int          pass_cnt;
  int          fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    chunk_imp  = new("chunk_imp",  this);
    result_imp = new("result_imp", this);
    void'(uvm_config_db #(bit)::get(this, "", "debug", debug));
    gold = crc_ref_dpi::type_id::create("gold");
    if (debug) shadow = crc_ref_sv::type_id::create("shadow");
  endfunction

  function void write_chunk(crc_chunk_item t);
    int unsigned g;
    int unsigned s;
    gold.update(t.ch, t.data);
    if (debug) shadow.update(t.ch, t.data);
    if (!t.last) return;

    g = gold.final_crc(t.ch);
    if (debug) begin
      s = shadow.final_crc(t.ch);
      if (g != s) `uvm_error("MODEL", $sformatf("ch%0d C model=%08h SV model=%08h", t.ch, g, s))
    end
    exp_q[t.ch].push_back(g);
  endfunction

  function void write_result(crc_result_item t);
    int unsigned e;
    if (exp_q[t.ch].size() == 0) begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("ch%0d: CRC %08h with no finished message", t.ch, t.crc))
      return;
    end
    e = exp_q[t.ch].pop_front();
    if (e == t.crc) pass_cnt++;
    else begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("ch%0d: exp=%08h act=%08h", t.ch, e, t.crc))
    end
  endfunction

  function void final_phase(uvm_phase phase);
    gold.cleanup();                 // free C heap
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("PASS=%0d FAIL=%0d", pass_cnt, fail_cnt), UVM_NONE)
  endfunction
endclass
