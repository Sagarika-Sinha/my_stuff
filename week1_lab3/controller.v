module fsm_controller (
    input clk, rst,
    input start, last, do_add,      
    
    // ---- TASK B3: NEW DFT SCAN PORTS ----
    input scan_enable,              
    input [2:0] scan_in,            
    output reg [2:0] state,         
    
    // Functional control outputs driven to other modules
    output reg load, add_en, shift, count_en, clear, done 
);

    // Explicit Binary State Assignments
    localparam IDLE  = 3'b000, // 0
               LOAD  = 3'b001, // 1
               CHECK = 3'b010, // 2
               ADD   = 3'b011, // 3
               SHIFT = 3'b100, // 4
               DONE  = 3'b101; // 5

    reg [2:0] next;

    // ALWAYS BLOCK 1: Next-State Combinational Logic
    always @(*) begin
        case (state)
            IDLE : next = start ? LOAD : IDLE;
            LOAD : next = CHECK;
            CHECK: next = do_add ? ADD : SHIFT; 
            ADD  : next = SHIFT;
            SHIFT: next = last ? DONE : CHECK;  
            DONE : next = IDLE;
            default: next = IDLE;
        endcase
    end

    // ALWAYS BLOCK 2: State Register with Scan Mux
    always @(posedge clk) begin
        if (rst) begin
            state <= IDLE; 
        end 
        else begin
            state <= scan_enable ? scan_in : next;
        end
    end

    // ALWAYS BLOCK 3: Moore Output Logic
    always @(*) begin
        // Reset defaults
        load     = 1'b0;
        clear    = 1'b0;
        add_en   = 1'b0;
        shift    = 1'b0;
        count_en = 1'b0;
        done     = 1'b0;
        
        case (state)
            LOAD : begin 
                load  = 1'b1; 
                clear = 1'b1; 
            end
            ADD  : begin
                add_en = 1'b1;
            end
            SHIFT: begin 
                shift    = 1'b1; 
                count_en = 1'b1; 
            end
            DONE : begin
                done = 1'b1;
            end
            default: ; 
        endcase
    end

endmodule