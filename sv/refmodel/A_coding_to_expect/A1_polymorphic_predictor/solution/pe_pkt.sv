// pe_pkt.sv : header object + packet item, both with handwritten deep methods
class pe_hdr extends uvm_object;
  `uvm_object_utils(pe_hdr)

  rand bit [7:0]  src;
  rand bit [7:0]  dst;
  bit      [15:0] len;
  bit      [7:0]  seq;

  function new(string name = "pe_hdr");
    super.new(name);
  endfunction

  function void do_copy(uvm_object rhs);
    pe_hdr rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "pe_hdr::do_copy wrong type")
    super.do_copy(rhs);
    src = rhs_c.src;
    dst = rhs_c.dst;
    len = rhs_c.len;
    seq = rhs_c.seq;
  endfunction

  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    pe_hdr rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    return (src == rhs_c.src) && (dst == rhs_c.dst) &&
           (len == rhs_c.len) && (seq == rhs_c.seq);
  endfunction

  function string convert2string();
    return $sformatf("src=%0h dst=%0h len=%0d seq=%0d", src, dst, len, seq);
  endfunction
endclass


class pe_pkt extends uvm_sequence_item;
  `uvm_object_utils(pe_pkt)

  rand pe_hdr    hdr;          // nested object -> deep copy needed
  rand bit [7:0] payload[];

  constraint c_len { payload.size() inside {[1:64]}; }

  function new(string name = "pe_pkt");
    super.new(name);
    hdr = pe_hdr::type_id::create("hdr");
  endfunction

  function void post_randomize();
    hdr.len = payload.size();
  endfunction

  // DEEP copy: new header object + element-by-element payload
  function void do_copy(uvm_object rhs);
    pe_pkt rhs_c;
    if (!$cast(rhs_c, rhs)) `uvm_fatal("CAST", "pe_pkt::do_copy wrong type")
    super.do_copy(rhs);
    hdr = pe_hdr::type_id::create("hdr");          // NOT: hdr = rhs_c.hdr (shallow!)
    hdr.copy(rhs_c.hdr);
    payload = new[rhs_c.payload.size()];
    for (int i = 0; i < rhs_c.payload.size(); i++)
      payload[i] = rhs_c.payload[i];
  endfunction

  function bit do_compare(uvm_object rhs, uvm_comparer comparer);
    pe_pkt rhs_c;
    if (!$cast(rhs_c, rhs)) return 0;
    if (!hdr.compare(rhs_c.hdr))                return 0;   // reuse pe_hdr::do_compare
    if (payload.size() != rhs_c.payload.size()) return 0;
    for (int i = 0; i < payload.size(); i++)
      if (payload[i] != rhs_c.payload[i]) return 0;
    return 1;
  endfunction

  function string convert2string();
    string s;
    s = $sformatf("[%s] payload(%0d):", hdr.convert2string(), payload.size());
    for (int i = 0; i < payload.size(); i++)
      s = {s, $sformatf(" %02h", payload[i])};
    return s;
  endfunction
endclass
