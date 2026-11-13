// timer_ref_model.sv : cycle-stepped model (step() = one posedge)
// Spec:
//   CTRL @0 : [0] EN, [1] AUTO_RELOAD, [15:8] PRESC
//   LOAD @4 : reload value; writing it also loads the counter
//   Each edge, in this order:
//     1. if EN: prescaler counts 0..PRESC; wrap = one "tick"
//     2. on tick: cnt==0 -> irq pulse, cnt = AUTO ? LOAD : 0, and EN=0 if !AUTO
//                 else    cnt--
//     3. bus write applied last (write wins over counting)
//   => irq period = (LOAD+1) * (PRESC+1) cycles
class timer_ref_model extends uvm_object;
  `uvm_object_utils(timer_ref_model)

  bit        en;
  bit        auto_rl;
  bit [7:0]  presc;
  bit [31:0] load;
  bit [31:0] cnt;
  bit [7:0]  presc_cnt;

  function new(string name = "timer_ref_model");
    super.new(name);
    reset();
  endfunction

  function void reset();
    en = 0; auto_rl = 0; presc = 0; load = 0; cnt = 0; presc_cnt = 0;
  endfunction

  function timer_item step(timer_item in);
    timer_item exp;
    bit tick;

    exp = timer_item::type_id::create("exp");
    exp.copy(in);
    exp.irq = 0;
    tick    = 0;

    // 1. prescaler
    if (en) begin
      if (presc_cnt == presc) begin
        presc_cnt = 0;
        tick      = 1;
      end
      else presc_cnt = presc_cnt + 1;
    end

    // 2. counter
    if (tick) begin
      if (cnt == 0) begin
        exp.irq = 1;
        if (auto_rl) cnt = load;
        else         en  = 0;
      end
      else cnt = cnt - 1;
    end

    // 3. bus write
    if (in.wr_en) begin
      case (in.addr)
        4'h0: begin
          if (!en && in.wdata[0]) presc_cnt = 0;   // fresh start on enable
          en      = in.wdata[0];
          auto_rl = in.wdata[1];
          presc   = in.wdata[15:8];
        end
        4'h4: begin
          load = in.wdata;
          cnt  = in.wdata;
        end
      endcase
    end
    return exp;
  endfunction
endclass
