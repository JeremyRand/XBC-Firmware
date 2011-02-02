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

// include primary source
`include "primary.v"

// Secondary module declaration  
module RedGreen1(CartData, CartAddr, CartCs, CartRd, CartWr, CartIReq, FData, FAddr, FCe, FOe, FWe, 
   PA, ClkInA, PB, ClkInB, GreenLED, RedLED, RCs, Clk);

   // Declare input/output ports -- these correspond to physical FPGA I/O pins
   inout  [15:0] CartData; // Multiplexed data/address bus from GBA cartridge
   input  [7:0] CartAddr;  // Most significant 8 bits of address bus from GBA cartridge
   input  CartCs;          // GBA cartridge select               
   input  CartRd;          // GBA cartridge read
   input  CartWr;          // GBA cartridge write
   output CartIReq;        // GBA cartridge interrupt request
   inout  [7:0] FData;     // Least significant 8 bits from flash
   output [20:0] FAddr;    // Flash address bits
   output FCe;             // Flash chip enable
   output FOe;             // Flash output enable
   output FWe;             // Flash write enable
   inout  [30:0] PA;       // PA external connector signals
   input  ClkInA;          // Global clock input A
   inout  [30:0] PB;       // PB external connector signals
   input  ClkInB;          // Global clock input B
   output GreenLED;        // Green LED control
   output RedLED;          // Red LED control
   output RCs;             // SDRAM chip select
   input  Clk;             // 50MHz clock

   reg    [15:0] DataRd;
   wire   [23:0] Addr;
   wire   Rd;
   wire   Wr;
   wire   Reset;

   // Instantiate primary
   Primary InstPrimary(.CartData(CartData), .CartAddr(CartAddr),  
      .CartCs(CartCs), .CartRd(CartRd), .CartWr(CartWr), .CartIReq(CartIReq), 
      .FData(FData), .FAddr(FAddr), .FCe(FCe), .FOe(FOe), .FWe(FWe), 
      .Addr(Addr), .Rd(Rd), .Wr(Wr), .SecDataRd(DataRd), .Clk(Clk));

   // The above code is essentially "boilerplate" (declare ports and instantiate Primary).
   // This code (or similar) must be included in each logic configuration.
   // The code below is actually doing what we want to do (turn the LEDs on and off).

   wire LEDEn;
   reg [1:0] LEDReg;

   // Create an "enable" signal for our LED logic that selects the address within
   // the GBA cartridge address space for our LED register.  Here we have chosen 0xffe200.  
   // Take this value, multiply by 2 and add 0x8000000 to get the actual address used by 
   // the GBA software (0x9ffc400).
   assign LEDEn = Addr[23:8]==16'hffe2;

   // Handle data write operations
   always @(negedge Wr)
      begin
      if (LEDEn)
         LEDReg <= CartData[1:0];
      end

   // Handle data read operations
   always @(LEDEn or LEDReg)
      begin
      if (LEDEn)
         DataRd = {14'b00000000000000, LEDReg[1:0]};
      else
         DataRd = 16'hxxxx;
      end

   // Assign LED states according to LEDReg
   assign RedLED = ~LEDReg[0];
   assign GreenLED = ~LEDReg[1];

   // disable SDRAM if available by disabling chip select
   assign RCs = 1'b1;
endmodule

