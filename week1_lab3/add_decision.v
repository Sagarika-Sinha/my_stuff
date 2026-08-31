// Student 3 — Conditional Add Logic (add_decision)
// Combinational. Inputs: Q_lsb, A, M.
// Outputs: sum = A+{1'b0,M} (always), do_add = Q_lsb.

module add_decision (
	input [8:0] A,
	input [7:0] M,
	input Q_lsb,
	output [8:0] sum,
	output do_add
);

    assign sum    = A + {1'b0, M};

    assign do_add = Q_lsb;

endmodule
