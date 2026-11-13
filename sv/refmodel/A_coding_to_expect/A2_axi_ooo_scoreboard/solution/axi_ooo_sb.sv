// axi_ooo_sb.sv : per-ID expected queues, three imps
`uvm_analysis_imp_decl(_ar)
`uvm_analysis_imp_decl(_r)
`uvm_analysis_imp_decl(_wr)

class axi_ooo_sb extends uvm_scoreboard;
  `uvm_component_utils(axi_ooo_sb)

  uvm_analysis_imp_ar #(axi_ar_item, axi_ooo_sb) ar_imp;
  uvm_analysis_imp_r  #(axi_r_item,  axi_ooo_sb) r_imp;
  uvm_analysis_imp_wr #(mem_wr_item, axi_ooo_sb) wr_imp;

  axi_ref_mem ref_mem;
  axi_r_item  exp_q[bit [3:0]][$];   // ID -> queue of expected beats
  int         pass_cnt;
  int         fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    ar_imp  = new("ar_imp", this);
    r_imp   = new("r_imp",  this);
    wr_imp  = new("wr_imp", this);
    ref_mem = axi_ref_mem::type_id::create("ref_mem");
  endfunction

  // ---- memory side ----
  function void write_wr(mem_wr_item t);
    ref_mem.write_word(t.addr, t.data);
  endfunction

  // ---- request: predict every beat now ----
  function void write_ar(axi_ar_item t);
    axi_ar_item ar;
    axi_r_item  exp;
    bit [31:0]  addr_q[$];

    // monitor may reuse 't' -> own copy before keeping anything from it
    if (!$cast(ar, t.clone())) `uvm_fatal("CAST", "clone failed")

    ref_mem.calc_beats(ar, addr_q);
    for (int i = 0; i < addr_q.size(); i++) begin
      exp      = axi_r_item::type_id::create("exp");   // NEW object per beat
      exp.id   = ar.id;
      exp.addr = addr_q[i];
      exp.data = ref_mem.read_word(addr_q[i]);
      exp.resp = 2'b00;
      exp.last = (i == addr_q.size() - 1);
      exp_q[ar.id].push_back(exp);
    end
  endfunction

  // ---- response: in order inside the ID only ----
  function void write_r(axi_r_item t);
    axi_r_item exp;
    if (!exp_q.exists(t.id) || exp_q[t.id].size() == 0) begin
      `uvm_error("SB", $sformatf("R beat with no outstanding AR: %s", t.convert2string()))
      fail_cnt++;
      return;
    end
    exp = exp_q[t.id].pop_front();
    if (exp.compare(t)) pass_cnt++;
    else begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("MISMATCH\n exp: %s\n act: %s",
                                 exp.convert2string(), t.convert2string()))
    end
    if (exp_q[t.id].size() == 0) exp_q.delete(t.id);   // keep map small
  endfunction

  function void check_phase(uvm_phase phase);
    foreach (exp_q[id])
      `uvm_error("SB", $sformatf("ID %0h: %0d beats never returned (next addr=%0h)",
                                 id, exp_q[id].size(), exp_q[id][0].addr))
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("PASS=%0d FAIL=%0d", pass_cnt, fail_cnt), UVM_NONE)
  endfunction
endclass
