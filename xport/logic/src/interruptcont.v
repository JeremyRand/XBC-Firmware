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

`ifndef _INTERRUPTCONT
`define _INTERRUPTCONT

module InterruptCont(Addr, DataRd, DataWr, En, Rd, Wr, IntStatus, IntReset, IntReq, Reset, Clk);
   
   input  [3:0] Addr;
   output [15:0] DataRd; 
   input  [15:0] DataWr;
   input  En;
   input  Rd;
   input  Wr;
   input  [15:0] IntStatus;
   output [15:0] IntReset;
   output IntReq;
   input  Reset;
   input  Clk;

   reg  [15:0] DataRd;
   reg  [15:0] IntMask;
   reg  [15:0] IntReset;
   reg  [3:0] AddrSync;
   reg  EnSync;
   reg  WrSync;
   reg  WrSyncPrev;
   wire [15:0] IntFilt;

   assign IntFilt = IntStatus&IntMask;
   assign IntReq = IntFilt!=0;
   assign WrPos = ~WrSyncPrev&WrSync;

   always @(posedge Clk)
      begin
      AddrSync <= Addr;
      EnSync <= En;
      WrSync <= Wr;
      WrSyncPrev <= WrSync;

      if (Reset)
         IntMask <= 0;
      else
         begin
         if (WrPos & EnSync)
            begin
            if (AddrSync==0)
               IntMask <= DataWr;
            else if (AddrSync==1)
               IntReset <= DataWr;
            end
         else
            IntReset <= 0; 
         end
      end

   always @(Addr or IntMask or IntFilt)
      begin
      if (Addr==0)
         DataRd = IntMask;
      else if (Addr==1)
         DataRd = IntFilt;
      else
         DataRd = 16'hxxxx;
      end

endmodule
    
`endif
