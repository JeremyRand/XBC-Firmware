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

module Pwm(Addr, DataRd, DataWr, En, Rd, Wr, P, Clk);

   parameter NUM_PWM = 2;
   
   input  [3:0] Addr;
   output [15:0] DataRd; 
   input  [15:0] DataWr;
   input  En;
   input  Rd;
   input  Wr;
   inout  [NUM_PWM-1:0] P;   
   input  Clk;

   reg     [15:0] DataRd;
   reg     [NUM_PWM*8-1:0] PwmReg;
   integer i;
   integer j;
             
   PwmLogic #(NUM_PWM) InstPwmLogic(PwmReg, P[NUM_PWM-1:0], Clk);
   
   always @(negedge Wr)
      begin
      for (i=0; i<NUM_PWM/2; i=i+1)
         begin
         if (Addr[3:0]==i && En)
            begin
            for (j=0; j<16; j=j+1)
               PwmReg[j+i*16] = DataWr[j];
            end
         end
      end

   always @(Addr or PwmReg)
      begin
      DataRd = 16'hxxxx;
      for (i=0; i<NUM_PWM/2; i=i+1)
         begin
         if (Addr[3:0]==i)
            begin
            for (j=0; j<16; j=j+1)
               DataRd[j] = PwmReg[j+i*16];
            end
         end
      end
   
endmodule
   
module PwmLogic(ChannelReg, ChannelOut, Clk);

   parameter NUM_PWM = 1;
  
   input  [NUM_PWM*8-1:0] ChannelReg;
   output [NUM_PWM-1:0] ChannelOut;
   input  Clk;

   reg     [NUM_PWM-1:0] ChannelOut;
   reg     [NUM_PWM-1:0] PreOut;
   reg	  [7:0] Counter;
   reg     [7:0] Channel;
   integer i;
   integer j;

   always @(posedge Clk)
      begin
      Counter = Counter + 1;
      for (i=0; i<NUM_PWM; i=i+1)
         ChannelOut[i] = PreOut[i];
      end

   always @(Counter or ChannelReg)
      begin
      for (i=0; i<NUM_PWM; i=i+1)
         begin
         for (j=0; j<8; j=j+1)
            Channel[j] = ChannelReg[j+i*8];            
         PreOut[i] = (Channel[7:0]>Counter[7:0]) | (Channel[7:0]==8'hff);
         end
      end
             
endmodule
 