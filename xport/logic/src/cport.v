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

`ifndef _CPORT
`define _CPORT

module Cport(Addr, DataRd, DataWr, En, Rd, Wr, CPDataRd, CPDataWr, CPDataDir, CPReady, CPReset, CPDir, CPStrobe, Clk);

   input  [3:0] Addr;
   output [15:0] DataRd; 
   input  [15:0] DataWr;
   input  En;
   input  Rd;
   input  Wr;
   output [3:0] CPDataRd;
   input  [3:0] CPDataWr;
   output CPDataDir;
   output CPReady;
   input  CPReset;
   input  CPDir;
   input  CPStrobe;
   input  Clk;

endmodule

`endif