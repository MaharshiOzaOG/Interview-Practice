// timer_scoreboard.sv : monitor sends one item EVERY cycle -> lock-step compare
class timer_scoreboard extends uvm_scoreboard;
  `uvm_component_utils(timer_scoreboard)

  uvm_analysis_imp #(timer_item, timer_scoreboard) mon_imp;

  timer_ref_model ref_m;
  int irq_seen;
  int fail_cnt;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    mon_imp = new("mon_imp", this);
    ref_m   = timer_ref_model::type_id::create("ref_m");
  endfunction

  function void write(timer_item t);
    timer_item exp;
    exp = ref_m.step(t);
    if (t.irq) irq_seen++;
    if (!exp.compare(t)) begin
      fail_cnt++;
      `uvm_error("SB", $sformatf("IRQ mismatch @cycle %0d: exp=%0b act=%0b (cnt=%0d presc_cnt=%0d)",
                                 t.cycle, exp.irq, t.irq, ref_m.cnt, ref_m.presc_cnt))
    end
  endfunction

  function void report_phase(uvm_phase phase);
    `uvm_info("SB", $sformatf("IRQs=%0d FAIL=%0d", irq_seen, fail_cnt), UVM_NONE)
  endfunction
endclass
