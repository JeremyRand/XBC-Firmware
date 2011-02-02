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

`ifndef _SDRAMCONT
`define _SDRAMCONT

module SdramCont(HAddr, HDataRd, HEn, HRd, HWr, HReset, HIdle, RData, RDataDir, RAddr, RBs, RClk, RCke, RCs, RWe, RRas, RCas, RLdqm, RUdqm);

   input  [22:0] HAddr;
   output [15:0] HDataRd;
   input  HEn;
   input  HRd;
   input  HWr;
   input  HReset;
   input  HIdle;
   input  [15:0] RData;
   output RDataDir;
   output [11:0] RAddr;
   output [1:0] RBs;
   input  RClk;
   output RCke;
   output RCs;
   output RWe;
   output RRas;
   output RCas;
   output RLdqm;
   output RUdqm;

endmodule

`endif