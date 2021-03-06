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

`ifndef _QUADRATURE
`define _QUADRATURE

//
// This code implements 2 phase quadrature signal decoding.  Quadrature signals are 
// often generated by optical shaft encoders for servo control applications.
//

//
// Topmost module -- instantiate this module
//
module Quadrature(Addr, DataRd, En, Rd, Wr, A, B, Clk);

   parameter NUM_QUADRA = 1;
   
   input  [3:0] Addr;
   output [15:0] DataRd; 
   input  En;
   input  Rd;
   input  Wr;
   input  [NUM_QUADRA-1:0] A;   
   input  [NUM_QUADRA-1:0] B;   
   input  Clk;

   reg     [15:0] DataRd;
   wire    [NUM_QUADRA*8-1:0] QuadraReg;
   integer i;
   integer j;
               
   QuadratureLogic #(NUM_QUADRA) InstQuadratureLogic(.InA(A), .InB(B), .Count(QuadraReg), .Clk(Clk));
      
   always @(Addr or QuadraReg or En)
      begin
      // infer multiplexer logic
      DataRd = 16'bx; 
      for (i=0; i<NUM_QUADRA; i=i+1)
         begin
         if (Addr[3:0]==i)
            begin
            for (j=0; j<8; j=j+1)
               DataRd[j] = QuadraReg[i*8+j];
            DataRd[15:8] = 8'h00;
            end
         end
      end
   
endmodule
   
module NoiseFilter(In, Out, Clk);

   parameter NUM_FILTER  = 1;
   parameter NUM_REJECT  = 2;
   parameter BITS_FILTER = 2;

   input   [NUM_FILTER-1:0] In;
   output  [NUM_FILTER-1:0] Out;
   input   Clk;

   reg     [NUM_FILTER-1:0] Out;
   reg     [BITS_FILTER-1:0] FilterCount [NUM_FILTER-1:0];
   integer i;

   always @(posedge Clk)
      begin
      for (i=0; i<NUM_FILTER; i=i+1)
         begin 
         if (In[i] != Out[i])
            begin           
            if (FilterCount[i] >= NUM_REJECT)
               begin
               FilterCount[i] <= 0;
               Out[i] <= In[i];
               end
            else
               FilterCount[i] <= FilterCount[i] + 1;
            end
         else
            FilterCount[i] <= 0;
         end  
      end
   endmodule

module QuadratureLogic(InA, InB, Count, Clk);

   parameter NUM_CHANNEL = 1;
   parameter WIDTH_COUNT = 8;

   input   [NUM_CHANNEL-1:0] InA;
   input   [NUM_CHANNEL-1:0] InB;
   output  [NUM_CHANNEL*WIDTH_COUNT-1:0] Count;
   input   Clk;
   
   wire    [NUM_CHANNEL-1:0] InAp;
   wire    [NUM_CHANNEL-1:0] InBp;
   reg     [NUM_CHANNEL*WIDTH_COUNT-1:0] Count;
   reg     [1:0] ChannelState [NUM_CHANNEL-1:0];
   reg     [WIDTH_COUNT-1:0] CountTemp;
   integer i;
   integer j;
  
   NoiseFilter #(NUM_CHANNEL*2, 40, 6) Inst1NoiseFilter({InA, InB}, {InAp, InBp}, Clk);

   always @(posedge Clk)
      begin
      for (i=0; i<NUM_CHANNEL; i=i+1)
         begin 
         for (j=0; j<WIDTH_COUNT; j=j+1)
            CountTemp[j] = Count[WIDTH_COUNT*i + j]; 
         case (ChannelState[i])
            2'b00: begin
               if ({InAp[i], InBp[i]} == 2'b01)
                  CountTemp = CountTemp + 1;
               else if ({InAp[i], InBp[i]} == 2'b10)
                  CountTemp = CountTemp - 1; 
               else 
                  CountTemp = CountTemp; 
               end 
            2'b01: begin
               if ({InAp[i], InBp[i]} == 2'b11)
                  CountTemp = CountTemp + 1;
               else if ({InAp[i], InBp[i]} == 2'b00)
                  CountTemp = CountTemp - 1; 
               else 
                  CountTemp = CountTemp; 
               end 
            2'b10: begin
               if ({InAp[i], InBp[i]} == 2'b00)
                  CountTemp = CountTemp + 1;
               else if ({InAp[i], InBp[i]} == 2'b11)
                  CountTemp = CountTemp - 1; 
               else 
                  CountTemp = CountTemp; 
               end 
            2'b11: begin
               if ({InAp[i], InBp[i]} == 2'b10)
                  CountTemp = CountTemp + 1;
               else if ({InAp[i], InBp[i]} == 2'b01)
                  CountTemp = CountTemp - 1; 
               else 
                  CountTemp = CountTemp; 
               end 
         endcase 
         for (j=0; j<WIDTH_COUNT; j=j+1)
            Count[WIDTH_COUNT*i + j] = CountTemp[j]; 
         ChannelState[i] <= {InAp[i], InBp[i]};
         end
      end
endmodule

`endif