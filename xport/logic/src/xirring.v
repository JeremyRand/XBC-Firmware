module XirRing(Addr, DataRd, DataWr, En, Rd, Wr, LedOut, SenseIn, Clk, Reset);

   parameter DIVISOR = 16'h02a9;
	parameter HALF_PERIOD = 8'h80; 
   
   input  [3:0] Addr;
   output [15:0] DataRd; 
   input  [15:0] DataWr;
   input  En;
   input  Rd;
   input  Wr;
   output [7:0] LedOut;
	input  [7:0] SenseIn;  
	input  Clk; 
	input  Reset;

endmodule