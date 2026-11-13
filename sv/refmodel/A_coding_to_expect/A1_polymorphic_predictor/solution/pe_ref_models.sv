// pe_ref_models.sv : abstract base + 3 policies
//   virtual base  -> cannot be created, only holds the contract + shared state
//   pe_ref_pass   -> concrete DEFAULT, this is the type the scoreboard creates
//   csum / split  -> extend pass, so they are legal factory overrides of it
virtual class pe_ref_base extends uvm_object;

  static int total_predicted;           // ONE copy for all model objects

  function new(string name = "pe_ref_base");
    super.new(name);
  endfunction

  pure virtual function void predict(pe_pkt in, ref pe_pkt out_q[$]);

  // helper every child uses: deep clone of the input
  protected function pe_pkt clone_pkt(pe_pkt in, string name);
    pe_pkt p;
    p = pe_pkt::type_id::create(name);
    p.copy(in);
    return p;
  endfunction
endclass


class pe_ref_pass extends pe_ref_base;
  `uvm_object_utils(pe_ref_pass)

  function new(string name = "pe_ref_pass");
    super.new(name);
  endfunction

  virtual function void predict(pe_pkt in, ref pe_pkt out_q[$]);
    out_q.push_back(clone_pkt(in, "exp"));
    total_predicted++;
  endfunction
endclass


class pe_ref_csum extends pe_ref_pass;
  `uvm_object_utils(pe_ref_csum)

  function new(string name = "pe_ref_csum");
    super.new(name);
  endfunction

  virtual function void predict(pe_pkt in, ref pe_pkt out_q[$]);
    pe_pkt    exp;
    bit [7:0] sum;
    int       n;

    exp = clone_pkt(in, "exp");
    sum = 0;
    n   = exp.payload.size();
    for (int i = 0; i < n; i++) sum = sum + exp.payload[i];

    exp.payload    = new[n + 1](exp.payload);   // grow, keep old bytes
    exp.payload[n] = sum;
    exp.hdr.len    = n + 1;

    out_q.push_back(exp);
    total_predicted++;
  endfunction
endclass


class pe_ref_split extends pe_ref_pass;
  `uvm_object_utils(pe_ref_split)

  int max_chunk = 16;

  function new(string name = "pe_ref_split");
    super.new(name);
  endfunction

  virtual function void predict(pe_pkt in, ref pe_pkt out_q[$]);
    pe_pkt exp;
    int    pos;
    int    sz;
    int    seq;

    pos = 0;
    seq = 0;
    while (pos < in.payload.size()) begin
      sz = in.payload.size() - pos;
      if (sz > max_chunk) sz = max_chunk;

      exp = pe_pkt::type_id::create($sformatf("exp_%0d", seq));
      exp.hdr.copy(in.hdr);                  // own header per chunk (deep)
      exp.hdr.len = sz;
      exp.hdr.seq = seq;
      exp.payload = new[sz];
      for (int i = 0; i < sz; i++) exp.payload[i] = in.payload[pos + i];

      out_q.push_back(exp);
      total_predicted++;
      pos = pos + sz;
      seq = seq + 1;
    end
  endfunction
endclass
