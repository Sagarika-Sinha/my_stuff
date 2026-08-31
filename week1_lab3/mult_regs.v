module mult_regs(input clk, input rst, input load, input [8:0] sum, input add_en, input shift, input [7:0] a_in, input [7:0]b_in,
input scan_enable, input [8:0] scan_in_A, input [7:0] scan_in_M, input [7:0] scan_in_Q,
output reg [8:0] A, output reg [7:0] M, output reg [7:0] Q);

    always @(posedge clk)
    begin 
        if (rst) begin 
            A<=9'b0;
            M<=8'b0;
            Q<=8'b0;
        end
        else if (scan_enable) begin
            A<=scan_in_A;
            M<=scan_in_M;
            Q<=scan_in_Q;
        end 
        else if (load) begin 
            M<=a_in;
            Q<=b_in;
            A<=9'b0;
        end
        else if (add_en) A<=sum;
        else if (shift) {A,Q}<={1'b0,A,Q[7:1]};
        else
        A<=A;
    end

endmodule