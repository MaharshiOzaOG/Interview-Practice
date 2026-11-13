// pipe_if.sv : counters live HERE -> every monitor/scoreboard sees the same time base
interface pipe_if (input logic clk, input logic rst_n);
  logic        in_valid,  in_ready;
  logic [31:0] in_data;
  logic        out_valid, out_ready;
  logic [31:0] out_data;

  longint cycle_cnt;
  longint stall_cnt;

  always @(posedge clk) begin
    if (!rst_n) begin
      cycle_cnt <= 0;
      stall_cnt <= 0;
    end
    else begin
      cycle_cnt <= cycle_cnt + 1;
      if (out_valid && !out_ready) stall_cnt <= stall_cnt + 1;
    end
  end
endinterface
