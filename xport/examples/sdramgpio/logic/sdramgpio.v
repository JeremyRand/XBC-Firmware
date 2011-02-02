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

`include "primary.v"
`include "gpio.v"
`include "sdramcont.v"

module SdramGpio(CartData, CartAddr, FData, FAddr, CartCs, CartRd, CartWr, CartIReq, FCe, FOe, FWe, 
   PA, PB, ClkInA, ClkInB, CPData, CPReady, CPReset, CPDir, CPStrobe, GreenLED, RedLED, 
   RData, RAddr, RBs, RCke, RCs, RWe, RRas, RCas, RLdqm, RUdqm, Clk, Phi);

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
   inout  [15:0] RData;
   output [11:0] RAddr;
   output [1:0] RBs;
   output RCke;
   output RCs;
   output RWe;
   output RRas;
   output RCas;
   output RLdqm;
   output RUdqm;
   input  Clk;
   input  Phi;

   reg    [15:0] DataRd;
   wire   [23:0] Addr;
   wire   Rd;
   wire   Wr;
   wire   Reset;
   Primary InstPrimary(.CartData(CartData), .CartAddr(CartAddr),  
      .CartCs(CartCs), .CartRd(CartRd), .CartWr(CartWr), .CartIReq(CartIReq), 
      .FData(FData), .FAddr(FAddr), .FCe(FCe), .FOe(FOe), .FWe(FWe), 
      .CPData(CPData), .CPReady(CPReady), .CPReset(CPReset), .CPDir(CPDir), .CPStrobe(CPStrobe), 
      .GreenLED(GreenLED), .RedLED(RedLED), .Addr(Addr), .Rd(Rd), .Wr(Wr), 
      .SecDataRd(DataRd), .Identifier(16'h8014), .Clk(Clk), .Reset(Reset));

   wire   GpioEn;
   wire   [15:0] GpioDataRd;
   assign GpioEn = Addr[23:8]==16'hffe2;
   Gpio #(32) InstGpio(.Addr(Addr[3:0]), .DataRd(GpioDataRd), .DataWr(CartData), 
      .En(GpioEn), .Rd(Rd), .Wr(Wr), .P({PB[0], PA[30:0]}));

   wire   RamEn;
   wire   [15:0] RamDataRd;
   wire   RDataDir;
   assign RamEn = Addr[23];
   assign RData = RDataDir ? CartData : 16'hzzzz;
   SdramCont InstSdram(.HAddr(Addr[22:0]), .HDataRd(RamDataRd),
      .HEn(RamEn), .HRd(Rd), .HWr(Wr), .HReset(Reset), .HIdle(~RamEn & Rd),
      .RData(RData), .RDataDir(RDataDir), .RAddr(RAddr), .RBs(RBs), .RClk(Clk), .RCke(RCke), .RCs(RCs), .RWe(RWe),
      .RRas(RRas), .RCas(RCas), .RLdqm(RLdqm), .RUdqm(RUdqm)); 
           
   always @(GpioEn or GpioDataRd or RamEn or RamDataRd)
      begin              
      if (GpioEn)
          DataRd = GpioDataRd;
      else if (RamEn)
          DataRd = RamDataRd;
      else
          DataRd = 16'hxxxx;
      end

endmodule

