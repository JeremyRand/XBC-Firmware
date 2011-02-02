module BemfCont4(Addr, DataRd, DataWr, En, Rd, Wr, PwmOut, PwmCont, AdcIn, AdcOut, AdcDir, AdcCs, AdcClk, IntStatus, IntReset, Reset, Clk, Active0, Measure0);

   input  [8:0] Addr;
   output [15:0] DataRd;
   input  [15:0] DataWr;
   input  En;
   input  Rd;
   input  Wr;
   output [3:0] PwmOut;
   output [7:0] PwmCont;
   input  AdcIn;
   output AdcOut;
	output AdcDir;
   output AdcCs;
   output AdcClk;
	output IntStatus;
	input  IntReset;
   input  Reset;
   input  Clk;
   output Active0;
   output Measure0;

endmodule