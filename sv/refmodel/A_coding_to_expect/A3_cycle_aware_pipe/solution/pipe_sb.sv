// pipe_sb.sv : untimed data predict + timing/credit/reset checks
`uvm_analysis_imp_decl(_in)
`uvm_analysis_imp_decl(_out)

class pipe_sb extends uvm_scoreboard;
  `uvm_component_utils(pipe_sb)

  uvm_analysis_imp_in  #(pipe_item, pipe_sb) in_imp;
  uvm_analysis_imp_out #(pipe_item, pipe_sb) out_imp;

  virtual pipe_if vif;
  bit [31:0]      key;
  int unsigned    min_lat  = 2;
  int unsigned    max_lat  = 10;
  int unsigned    credits  = 4;
  int unsigned    drain_to = 1000;   // cycles

  pipe_exp exp_q[$];
  int      pass_cnt;
  int      fail_cnt;
  int      flushed;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    in_imp  = new("in_imp",  this);
    out_imp = new("out_imp", this);
    if (!uvm_config_db #(virtual pipe_if)::get(this, "", "vif", vif))
      `uvm_fatal("NOVIF", "pipe_if not set")
    if (!uvm_config_db #(bit [31:0])::get(this, "", "key", key))
      `uvm_fatal("NOKEY", "key not set")
    void'(uvm_config_db #(int unsigned)::get(this, "", "min_lat", min_lat));
    void'(uvm_config_db #(int unsigned)::get(this, "", "max_lat", max_lat));
    void'(uvm_config_db #(int unsigned)::get(this, "", "credits", credits));
  endfunction

  // ---- untimed data model ----
  function bit [31:0] model(bit [31:0] d);
    return {d[15:0], d[31:16]} ^ key;
  endfunction

  // ---- input accepted ----
  function void write_in(pipe_item t);
    pipe_exp e;
    if (exp_q.size() >= credits)
      `uvm_error("CREDIT", $sformatf("DUT accepted beyond %0d credits @cyc %0d", credits, t.cycle))

    e           = new();
    e.item      = pipe_item::type_id::create("exp");
    e.item.data = model(t.data);
    e.in_cycle  = t.cycle;
    e.in_stall  = t.stall;
    exp_q.push_back(e);
  endfunction

  // ---- output seen ----
  function void write_out(pipe_item t);
    pipe_exp e;
    longint  lat;
    if (exp_q.size() == 0) begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("Output with nothing in flight: %s", t.convert2string()))
      return;
    end
    e   = exp_q.pop_front();
    lat = (t.cycle - e.in_cycle) - (t.stall - e.in_stall);

    if (!e.item.compare(t)) begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("DATA exp=%0h act=%0h", e.item.data, t.data))
    end
    else if (lat < min_lat || lat > max_lat) begin
      fail_cnt++;
      `uvm_error("LAT", $sformatf("latency %0d not in [%0d:%0d] (in@%0d out@%0d)",
                                  lat, min_lat, max_lat, e.in_cycle, t.cycle))
    end
    else pass_cnt++;
  endfunction

  // ---- watchdog + reset flush ----
  task run_phase(uvm_phase phase);
    longint age;
    forever begin
      @(posedge vif.clk);
      if (!vif.rst_n) begin
        if (exp_q.size() != 0) begin
          `uvm_info("SB", $sformatf("Reset: flushing %0d in-flight packets", exp_q.size()), UVM_LOW)
          flushed = flushed + exp_q.size();
          exp_q.delete();
        end
        continue;
      end
      if (exp_q.size() != 0 && !exp_q[0].timeout_reported) begin
        age = (vif.cycle_cnt - exp_q[0].in_cycle) - (vif.stall_cnt - exp_q[0].in_stall);
        if (age > max_lat) begin
          exp_q[0].timeout_reported = 1;
          fail_cnt++;
          `uvm_error("LOST", $sformatf("Packet accepted @%0d still not out after %0d cycles",
                                       exp_q[0].in_cycle, age))
        end
      end
    end
  endtask

  // ---- do not let run end while packets are in flight ----
  function void phase_ready_to_end(uvm_phase phase);
    if (phase.get_name() != "run") return;
    if (exp_q.size() == 0) return;
    phase.raise_objection(this, "draining pipe");
    fork
      begin
        int unsigned n;
        n = 0;
        while (exp_q.size() != 0 && n < drain_to) begin
          @(posedge vif.clk);
          n++;
        end
        if (exp_q.size() != 0)
          `uvm_error("DRAIN", $sformatf("%0d packets stuck at end of test", exp_q.size()))
        phase.drop_objection(this, "pipe drained");
      end
    join_none
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("PASS=%0d FAIL=%0d flushed_on_reset=%0d",
                              pass_cnt, fail_cnt, flushed), UVM_NONE)
  endfunction
endclass
