module BtUart(Addr, DataRd, DataWr, En, Rd, Wr, In, Out, Cts, Rts, IntStatus, IntReset, BtReset, Clk, Reset);

   input  [1:0] Addr;
   output [15:0] DataRd;
   input  [15:0] DataWr;
   input  En;
   input  Rd;
   input  Wr;
   input  In;
   output Out;
   input  Cts;
   output Rts;
   output IntStatus;
   input  IntReset;
   output BtReset;
   input  Clk;
   input  Reset;

endmodule