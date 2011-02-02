//  BEGIN LICENSE BLOCK
// 
//  Version: MPL 1.1
// 
//  The contents of this file are subject to the Mozilla Public License Version
//  1.1 (the "License"); you may not use this file except in compliance with
//  the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
// 
//  Software distributed under the License is distributed on an "AS IS" basis,
//  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
//  for the specific language governing rights and limitations under the
//  License.
// 
//  The Original Code is The Xport Logic Library.
// 
//  The Initial Developer of the Original Code is Charmed Labs LLC.
//  Portions created by the Initial Developer are Copyright (C) 2003
//  the Initial Developer. All Rights Reserved.
//             
//  Contributor(s): Rich LeGrand (rich@charmedlabs.com)
// 
//  END LICENSE BLOCK 

`include "primaryint.v"
`include "xirring.v"
`include "bemfcont4.v"
`include "btuart.v"
module Robot1(CartData, CartAddr, FData, FAddr, CartCs, CartRd, CartWr, CartIReq, FCe, FOe, FWe, 
   PA, PB, ClkInA, ClkInB, CPData, CPReady, CPReset, CPDir, CPStrobe, GreenLED, RedLED, RCs, Clk, Phi);

   inout  [15:0] CartData; 
   input  [7:0] CartAddr;
   inout  [7:0] FData;
   output [20:0] FAddr;
   input  CartCs;                         
   input  CartRd;
   input  CartWr;
   output CartIReq;
   output FCe;
   output FOe;
   output FWe;
   inout  [30:0] PA;
   inout  [30:0] PB;
   input  ClkInA;
   input  ClkInB;
   inout  [3:0] CPData;
   output CPReady;
   input  CPReset;
   input  CPDir;
   input  CPStrobe;
   output GreenLED;
   output RedLED;
   output RCs;
   input  Clk;
   input  Phi;

   reg  [15:0] DataRd;
   wire [15:0] UartDataRd;
   wire UartEn;
   wire [23:0] Addr;
   wire Rd;
   wire Wr;
   wire Reset;
   wire [13:0] Dummy;
   wire BtIntStatus;
   wire BtIntReset;
   wire BemfIntStatus;
   wire BemfIntReset;
	wire   Btx;

	// Primary

   PrimaryInt InstPrimaryInt(.CartData(CartData), .CartAddr(CartAddr),  
      .CartCs(CartCs), .CartRd(CartRd), .CartWr(CartWr), .CartIReq(CartIReq), 
      .FData(FData), .FAddr(FAddr), .FCe(FCe), .FOe(FOe), .FWe(FWe), 
      .CPData(CPData), .CPReady(CPReady), .CPReset(CPReset), .CPDir(CPDir), .CPStrobe(CPStrobe), 
      .GreenLED(GreenLED), .RedLED(RedLED), .Addr(Addr), .Rd(Rd), .Wr(Wr), 
      .SecDataRd(DataRd), .Identifier(16'h8c01), 
		.IntStatus({14'h0000, BtIntStatus, BemfIntStatus}), .IntReset({Dummy, BtIntReset, BemfIntReset}), .Clk(Clk), .Reset(Reset));

	// Back-EMF controller

   wire BemfEn;
   wire [15:0] BemfDataRd;
   wire [3:0] PwmOut;
   wire [7:0] PwmCont;
	wire BemfAdcDir;
	wire BemfAdcOut;

	assign PA[24] = BemfAdcDir ? BemfAdcOut : 1'bz;
   assign BemfEn = Addr[23:9]==15'h7ff1; //15'hffe2;

   BemfCont4 InstBemfCont(.Addr(Addr[8:0]), .DataRd(BemfDataRd), .DataWr(CartData),
      .En(BemfEn), .Rd(Rd), .Wr(Wr), 
      .PwmOut(PwmOut), .PwmCont(PwmCont), .AdcIn(PA[24]), .AdcOut(BemfAdcOut), .AdcDir(BemfAdcDir), .AdcCs(PA[25]), .AdcClk(PA[27]),
		.IntStatus(BemfIntStatus), .IntReset(BemfIntReset),  
      .Reset(Reset), .Clk(Clk)); // .Measure0(PA[2]), .Active0(PA[3]));

   assign PA[16] = ~(PwmCont[0]&PwmOut[0]);
   assign PA[17] = ~(PwmCont[1]&PwmOut[0]);
   
   assign PA[18] = ~(PwmCont[2]&PwmOut[1]);
   assign PA[19] = ~(PwmCont[3]&PwmOut[1]);

   assign PA[20] = ~(PwmCont[4]&PwmOut[2]);
   assign PA[21] = ~(PwmCont[5]&PwmOut[2]);

   assign PA[22] = ~(PwmCont[6]&PwmOut[3]);
   assign PA[23] = ~(PwmCont[7]&PwmOut[3]);

   // Bluetooth

	IBUFG InstIBUFG(.I(ClkInA), .O(Btx));

   assign UartEn = Addr[23:8]==16'hffe8;
   BtUart InstBtUart(.Addr(Addr[1:0]), .DataRd(UartDataRd), .DataWr(CartData), .En(UartEn), .Rd(Rd), .Wr(Wr), 
      .In(Btx), .Out(PA[26]), .Cts(PA[28]), .Rts(PA[29]), .IntStatus(BtIntStatus), .IntReset(BtIntReset), .BtReset(PA[30]), .Clk(Clk), .Reset(Reset));

   // XIR sensors

   wire   ProxEn;
	wire   [15:0] ProxDataRd;
   assign ProxEn = Addr[23:8]==16'hffe9;
	wire   [7:0] LedOut;
	XirRing InstXirRing(.Addr(Addr[3:0]), .DataRd(ProxDataRd), .DataWr(CartData), .En(ProxEn), .Rd(Rd), .Wr(Wr),
	   .LedOut(LedOut), .SenseIn({PA[14], PA[12], PA[10], PA[8], PA[6], PA[4], PA[2], PA[0]}), .Clk(Clk), .Reset(Reset));

	reg    [7:0] Mode;
	reg    [15:0] Ddr;
	reg    [15:0] Data;

	assign PA[0]  = Mode[0] ? 1'bz      : (Ddr[0]  ? Data[0]  : 1'bz);
	assign PA[1]  = Mode[0] ? LedOut[0] : (Ddr[1]  ? Data[1]  : 1'bz);
	assign PA[2]  = Mode[1] ? 1'bz      : (Ddr[2]  ? Data[2]  : 1'bz);
	assign PA[3]  = Mode[1] ? LedOut[1] : (Ddr[3]  ? Data[3]  : 1'bz); 
	assign PA[4]  = Mode[2] ? 1'bz      : (Ddr[4]  ? Data[4]  : 1'bz);
	assign PA[5]  = Mode[2] ? LedOut[2] : (Ddr[5]  ? Data[5]  : 1'bz); 
	assign PA[6]  = Mode[3] ? 1'bz      : (Ddr[6]  ? Data[6]  : 1'bz); 
	assign PA[7]  = Mode[3] ? LedOut[3] : (Ddr[7]  ? Data[7]  : 1'bz); 
	assign PA[8]  = Mode[4] ? 1'bz      : (Ddr[8]  ? Data[8]  : 1'bz);
	assign PA[9]  = Mode[4] ? LedOut[4] : (Ddr[9]  ? Data[9]  : 1'bz); 
	assign PA[10] = Mode[5] ? 1'bz      : (Ddr[10] ? Data[10] : 1'bz); 
	assign PA[11] = Mode[5] ? LedOut[5] : (Ddr[11] ? Data[11] : 1'bz); 
	assign PA[12] = Mode[6] ? 1'bz      : (Ddr[12] ? Data[12] : 1'bz); 
	assign PA[13] = Mode[6] ? LedOut[6] : (Ddr[13] ? Data[13] : 1'bz); 
	assign PA[14] = Mode[7] ? 1'bz      : (Ddr[14] ? Data[14] : 1'bz); 
	assign PA[15] = Mode[7] ? LedOut[7] : (Ddr[15] ? Data[15] : 1'bz);

	always @(negedge Wr)
	   begin
		if (ProxEn)
		   begin
			if (Addr[3:0]==10)
			   Mode <= CartData[7:0];
         else if (Addr[3:0]==11)
			   Ddr <= CartData;
         else if (Addr[3:0]==12)
			   Data <= CartData; 
         end
		end

	// Data bus mux
  
   always @(BemfEn or BemfDataRd or UartEn or UartDataRd or ProxEn or ProxDataRd)
      begin              
      if (BemfEn)
         DataRd = BemfDataRd;
      else if (UartEn)
         DataRd = UartDataRd;
      else if (ProxEn)
		   begin
			if (Addr[3:0]==10)
			   DataRd = {8'h00, Mode};
         else if (Addr[3:0]==11)
			   DataRd = Ddr;
         else if (Addr[3:0]==12)
			   DataRd = PA[15:0];
         else
			   DataRd = ProxDataRd;
        	end
      else
         DataRd = 16'hxxxx;
      end

   // disable SDRAM if available
   assign RCs = 1'b1;
endmodule

