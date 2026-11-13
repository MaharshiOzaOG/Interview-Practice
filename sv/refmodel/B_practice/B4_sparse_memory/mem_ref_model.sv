// mem_ref_model.sv : sparse byte memory (assoc array keyed by byte address)
//   only touched bytes exist -> 4 GB space costs nothing
//   uninit read policy: ZERO, ALL_X, or ERROR (report + return 0)
typedef enum {UNINIT_ZERO, UNINIT_X, UNINIT_ERROR} uninit_e;

class mem_ref_model extends uvm_object;
  `uvm_object_utils(mem_ref_model)

  logic [7:0] mem[bit [31:0]];
  uninit_e    policy;

  function new(string name = "mem_ref_model");
    super.new(name);
    policy = UNINIT_ZERO;
  endfunction

  function void write_byte(bit [31:0] a, bit [7:0] d);
    mem[a] = d;
  endfunction

  function logic [7:0] read_byte(bit [31:0] a);
    if (mem.exists(a)) return mem[a];
    case (policy)
      UNINIT_ZERO : return 8'h00;
      UNINIT_X    : return 8'hxx;
      UNINIT_ERROR: begin
        `uvm_error("MEM", $sformatf("Read of uninitialized byte %0h", a))
        return 8'h00;
      end
    endcase
    return 8'h00;
  endfunction

  function mem_item predict(mem_item in);
    mem_item exp;
    exp = mem_item::type_id::create("exp");
    exp.copy(in);

    if (in.write) begin
      for (int i = 0; i < 4; i++)
        if (in.strb[i]) write_byte(in.addr + i, in.wdata[8*i +: 8]);   // little endian
    end
    else begin
      for (int i = 0; i < 4; i++)
        exp.rdata[8*i +: 8] = read_byte(in.addr + i);
    end
    return exp;
  endfunction

  // backdoor preload, e.g. from a test
  function void preload(bit [31:0] base, bit [7:0] bytes[]);
    for (int i = 0; i < bytes.size(); i++)
      write_byte(base + i, bytes[i]);
  endfunction
endclass
