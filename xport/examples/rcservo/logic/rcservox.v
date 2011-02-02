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
`include "rcservo.v"

module RCServox(CartData, CartAddr, FData, FAddr, CartCs, CartRd, CartWr, CartIReq, FCe, FOe, FWe, 
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

   wire   [15:0] DataRd;
   wire   [23:0] Addr;
   wire   Rd;
   wire   Wr;

   Primary InstPrimary(.CartData(CartData), .CartAddr(CartAddr),  
      .CartCs(CartCs), .CartRd(CartRd), .CartWr(CartWr), .CartIReq(CartIReq), 
      .FData(FData), .FAddr(FAddr), .FCe(FCe), .FOe(FOe), .FWe(FWe), 
      .CPData(CPData), .CPReady(CPReady), .CPReset(CPReset), .CPDir(CPDir), .CPStrobe(CPStrobe), 
      .GreenLED(GreenLED), .RedLED(RedLED), .Addr(Addr), .Rd(Rd), .Wr(Wr), 
      .SecDataRd(DataRd), .Identifier(16'h8016), .Clk(Clk));

   wire   RCServoEn;
   assign RCServoEn = Addr[23:8]==16'hffe2;
   RCServo #(62) InstRCServo0(.Addr(Addr[4:0]), .DataRd(DataRd), .DataWr(CartData), 
      .En(RCServoEn), .Rd(Rd), .Wr(Wr), .P({PB[30:0], PA[30:0]}), .Clk(Phi));

   // disable SDRAM if available
   assign RCs = 1'b1;
endmodule

