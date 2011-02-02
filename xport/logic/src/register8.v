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

`ifndef _REGISTER8
`define _REGISTER8

module Register8(DataRd, DataWr, En, Rd, Wr, RegOut);

   output [7:0] DataRd;
   input  [7:0] DataWr;
   input  En;
   input  Rd;
   input  Wr;
   output [7:0] RegOut;

   reg [7:0] RegData;

   always @(negedge Wr)
      begin
      if (En)
         RegData <= DataWr[7:0];
      end

   assign DataRd = {8'h00, RegData};

   assign RegOut = RegData;

endmodule

`endif