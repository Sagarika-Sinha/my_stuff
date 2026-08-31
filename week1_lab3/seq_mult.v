module seq_multiplier (
  input clk,
  input rst,
  input start,
  input [7:0] a,
  input [7:0] b,
  output [15:0] product,
  output done,
  input scan_enable,
  input test_mode,
  input scan_in,
  output scan_out
);

  wire [8:0] A;
  wire [7:0] M;
  wire [7:0] Q;
  wire [2:0] state;
  wire [8:0] sum;
  wire [3:0] count;
  wire last;
  wire do_add;
  wire load;
  wire add_en;
  wire shift;
  wire count_en;
  wire clear;

  wire [2:0] scan_state_in; //for 6 states in fsm_controller 3 flipflop
  assign scan_state_in[0] = scan_in;     //scan_in --> controller.state[2:0] 
  assign scan_state_in[1] = state[0];     // chain of flipflop
  assign scan_state_in[2] = state[1];

  wire [8:0] scan_A_in; 
  // 9 bits = 9 flipflop converting every ff to scan-ff

  //controller.state[2:0] --> mult_regs.A[8:0]

  assign scan_A_in[0] = state[2];
  assign scan_A_in[1] = A[0];
  assign scan_A_in[2] = A[1];
  assign scan_A_in[3] = A[2];
  assign scan_A_in[4] = A[3];
  assign scan_A_in[5] = A[4];
  assign scan_A_in[6] = A[5];
  assign scan_A_in[7] = A[6];
  assign scan_A_in[8] = A[7];

  wire [7:0] scan_M_in;

  //mult_regs.A[8:0] --> mult_regs.M[7:0]

  assign scan_M_in[0] = A[8];
  assign scan_M_in[1] = M[0];
  assign scan_M_in[2] = M[1];
  assign scan_M_in[3] = M[2];
  assign scan_M_in[4] = M[3];
  assign scan_M_in[5] = M[4];
  assign scan_M_in[6] = M[5];
  assign scan_M_in[7] = M[6];

  wire [7:0] scan_Q_in;

  //mult_regs.M[7:0] --> mult_regs.Q[7:0]

  assign scan_Q_in[0] = M[7];
  assign scan_Q_in[1] = Q[0];
  assign scan_Q_in[2] = Q[1];
  assign scan_Q_in[3] = Q[2];
  assign scan_Q_in[4] = Q[3];
  assign scan_Q_in[5] = Q[4];
  assign scan_Q_in[6] = Q[5];
  assign scan_Q_in[7] = Q[6];
 
  wire [3:0] scan_count_in;

  //mult_regs.Q[7:0] --> count[3:0] 

  assign scan_count_in[0] = Q[7];
  assign scan_count_in[1] = count[0];
  assign scan_count_in[2] = count[1];
  assign scan_count_in[3] = count[2];


  assign scan_out = count[3];

  //count[3:0] --> scan_out


  mult_regs U1 (
    .clk(clk),
    .rst(rst),
    .load(load),
    .add_en(add_en),
    .sum(sum),
    .shift(shift),
    .a_in(a),
    .b_in(b),
    .scan_enable(scan_enable),
    .scan_in_A(scan_A_in),
    .scan_in_M(scan_M_in),
    .scan_in_Q(scan_Q_in),
    .A(A),
    .M(M),
    .Q(Q)
  ); 

  bit_counter U2 (
    .clk(clk),
    .rst(rst),
    .clear(clear),
    .count_en(count_en),
    .scan_enable(scan_enable),
    .test_mode(test_mode),
    .scan_in(scan_count_in),
    .count(count),
    .last(last)
  );

  add_decision U3 (
    .A(A),
    .M(M),
    .Q_lsb(Q[0]),
    .sum(sum),
    .do_add(do_add)
  );

  fsm_controller U4 (
    .clk(clk),
    .rst(rst),
    .start(start),
    .last(last),
    .do_add(do_add),
    .scan_enable(scan_enable),
    .scan_in(scan_state_in),
    .state(state),
    .load(load),
    .add_en(add_en),
    .shift(shift),
    .count_en(count_en),
    .clear(clear),
    .done(done)
  );

  assign product = {A[7:0], Q};

endmodule