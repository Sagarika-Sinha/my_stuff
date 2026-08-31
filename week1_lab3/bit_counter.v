module ICG (
    input clk,
    input enable,
    input test_enable,
    output gated_clk
);
    reg en_latch;
    always @(clk or enable or test_enable)
        if (!clk)
            en_latch = enable | test_enable;
    assign gated_clk = clk & en_latch;
endmodule



module bit_counter (
    input clk, rst,
    input clear, count_en,
    input test_mode,
    input scan_enable,
    input [3:0] scan_in,
    output reg [3:0] count,
    output last
);

    wire gated_clk;
    ICG u_icg (
        .clk        (clk),
        .enable     (count_en),
        .test_enable(test_mode),
        .gated_clk  (gated_clk)
    );

    wire [3:0] count_next_func;
    assign count_next_func = (clear) ? 4'b0 : count + 1'b1;

    always @(posedge gated_clk) begin
        if (rst)
            count <= 4'b0;
        else
            count <= scan_enable ? scan_in : count_next_func;
    end

    assign last = (count == 4'd7);

endmodule