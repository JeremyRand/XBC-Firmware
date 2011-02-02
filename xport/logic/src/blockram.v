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

`ifndef _BLOCKRAM
`define _BLOCKRAM

 module BlockRam(Addr, DataRd, DataWr, En, Rd, Wr, Clk);

   input  [7:0] Addr;
   output [15:0] DataRd;
   input  [15:0] DataWr;
   input  En;
   input  Rd;
   input  Wr;
   input  Clk;

   reg SyncEn;
   reg SyncWr;

   always @(posedge Clk)
      begin
      SyncEn <= En;
      SyncWr <= Wr;
      end

   RAMB4_S16 Ram0(.WE(SyncWr), .EN(SyncEn), .RST(1'b0), .ADDR(Addr), .CLK(Clk), .DI(DataWr), .DO(DataRd));

endmodule

`endif