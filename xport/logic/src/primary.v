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

`ifndef _PRIMARY
`define _PRIMARY

`include "cport.v"
`include "decodebus.v"
`include "blockram.v"

//
// This Primary module includes the following facilities:
//
// Module                     Base location (GBA memory)
// -----------------------------------------------------
// BlockRam (512 bytes)       0x9ffc000 
// Cport                      0x9ffc200
// LED control                0x9ffc220
// Indentifier                0x9ffc3e0  
//
module Primary(CartData, CartAddr, CartCs, CartRd, CartWr, CartIReq, FData, FAddr,FCe, FOe, FWe, CPData, CPReady, CPReset, CPDir, CPStrobe, GreenLED, RedLED, Addr, Rd, Wr, SecDataRd, Identifier, Clk, Reset);

   inout  [15:0] CartData; 
   input  [7:0] CartAddr;
   input  CartCs;
   input  CartRd;
   input  CartWr;
   output CartIReq;
   inout  [7:0] FData;
   output [20:0] FAddr;
   output FCe;
   output FOe;
   output FWe;
   inout  [3:0] CPData;
   output CPReady;
   input  CPReset;
   input  CPDir;
   input  CPStrobe;
   output GreenLED;
   output RedLED;
   output [23:0] Addr;
   output Rd;
   output Wr;
   input  [15:0] SecDataRd;
   input  [15:0] Identifier;
   input  Clk;
   output Reset;
   

   // decode GBA bus (boilerplate)
   reg  [15:0] DataRd;
   DecodeBus InstDecodeBus(.CartData(CartData), .CartAddr(CartAddr), .CartCs(CartCs), 
      .CartRd(CartRd), .CartWr(CartWr), .Addr(Addr), .Rd(Rd), .Wr(Wr));
   assign FData[7:0] = FWe ? 8'hzz : CartData[7:0];
   assign FAddr = Addr[20:0];
   assign FCe = Addr[23] | CartCs;  
   assign FOe = ~Rd;                
   assign FWe = ~Wr;               
   assign CartData[7:0] = Rd ? (Addr[23] ? DataRd[7:0] : FData) : 8'hzz;
   assign CartData[15:8] = Rd ? (Addr[23] ? DataRd[15:8] : 8'hzz) : 8'hzz;

   // Cport 
   wire CportEn;
   wire [15:0] CportDataRd;
   wire [3:0] CPDataRd;
   wire CPDataDir;
   Cport InstCport(.Addr(Addr[3:0]), .DataRd(CportDataRd), .DataWr(CartData), .En(CportEn), 
      .Rd(Rd), .Wr(Wr), .CPDataRd(CPDataRd), .CPDataWr(CPData), .CPDataDir(CPDataDir), 
      .CPReady(CPReady), .CPReset(CPReset), .CPDir(CPDir), .CPStrobe(CPStrobe), .Clk(Clk));
   assign CportEn = Addr[23:4]==20'hffe10;
   assign CPData = CPDataDir ? 4'hz : CPDataRd;

   // Block ram at 0x9ffc000
   wire RAMEn;
   wire [15:0] RAMDataRd;
   BlockRam InstRAM(.Addr(Addr[7:0]), .DataRd(RAMDataRd), .DataWr(CartData), 
      .En(RAMEn), .Rd(Rd), .Wr(Wr), .Clk(Clk));
   assign RAMEn = Addr[23:8]==16'hffe0;

   // LED control 
   reg  [1:0] LEDReg;
   wire LEDEn;
   always @(negedge Wr)
      begin
      if (LEDEn)
         LEDReg <= CartData[1:0];
      end
   assign LEDEn = Addr[23:4]==20'hffe11;
   assign GreenLED = ~LEDReg[0];
   assign RedLED = ~LEDReg[1];

   // Identifier
   wire IdentEn;
   assign IdentEn = Addr[23:4]==20'hffe1f;

   // power-up reset (synchronous)
   reg  [1:0] ResetCount;
   always @(posedge Clk)
      begin
      if (ResetCount!=2'b11)
         ResetCount <= ResetCount + 1;
      end
   assign Reset = ResetCount!=2'b11;  // active high

   // Combine data
   always @(CportEn or CportDataRd or RAMEn or IdentEn or RAMDataRd or SecDataRd or Identifier)
      begin
      if (RAMEn)
         DataRd = RAMDataRd;
      else if (CportEn)
         DataRd = CportDataRd;
      else if (IdentEn)
         DataRd = Identifier;
      else 
         DataRd = SecDataRd;
      end

   // No interrupt controller
   assign CartIReq = 1'b0;

endmodule

`endif
