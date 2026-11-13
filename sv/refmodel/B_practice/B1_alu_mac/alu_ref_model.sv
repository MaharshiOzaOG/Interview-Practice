// alu_ref_model.sv : untimed predictor. Keeps MAC accumulator as state.
//   8-bit ops : result[7:0], flags C V Z N
//   MAC       : acc = sat16( acc + signed(a)*signed(b) ), sat=1 if clipped
//   CLR_ACC   : acc = 0
class alu_ref_model extends uvm_object;
  `uvm_object_utils(alu_ref_model)

  bit signed [15:0] acc;   // model state

  function new(string name = "alu_ref_model");
    super.new(name);
    acc = 0;
  endfunction

  function void reset();
    acc = 0;
  endfunction

  function alu_item predict(alu_item in);
    alu_item          exp;
    bit [8:0]         sum9;
    bit signed [15:0] prod;
    bit signed [16:0] sum17;

    exp = alu_item::type_id::create("exp");
    exp.copy(in);
    exp.result = 0; exp.c = 0; exp.v = 0; exp.sat = 0;

    case (in.op)
      alu_item::ADD: begin
        sum9          = {1'b0, in.a} + {1'b0, in.b};
        exp.result[7:0] = sum9[7:0];
        exp.c         = sum9[8];
        exp.v         = (in.a[7] == in.b[7]) && (sum9[7] != in.a[7]);
      end
      alu_item::SUB: begin
        exp.result[7:0] = in.a - in.b;
        exp.c         = (in.a >= in.b);                               // no borrow
        exp.v         = (in.a[7] != in.b[7]) && (exp.result[7] != in.a[7]);
      end
      alu_item::AND_OP: exp.result[7:0] = in.a & in.b;
      alu_item::OR_OP : exp.result[7:0] = in.a | in.b;
      alu_item::XOR_OP: exp.result[7:0] = in.a ^ in.b;
      alu_item::MAC: begin
        prod  = $signed(in.a) * $signed(in.b);
        sum17 = acc + prod;                                           // 17 bits: no wrap
        if (sum17 > 17'sd32767) begin
          acc = 16'sh7FFF; exp.sat = 1;
        end
        else if (sum17 < -17'sd32768) begin
          acc = 16'sh8000; exp.sat = 1;
        end
        else begin
          acc = sum17[15:0];
        end
        exp.result = acc;
      end
      alu_item::CLR_ACC: begin
        acc        = 0;
        exp.result = 0;
      end
    endcase

    // Z/N look at the width the op produces
    if (in.op == alu_item::MAC || in.op == alu_item::CLR_ACC) begin
      exp.z = (exp.result == 0);
      exp.n = exp.result[15];
    end
    else begin
      exp.z = (exp.result[7:0] == 0);
      exp.n = exp.result[7];
    end
    return exp;
  endfunction
endclass
