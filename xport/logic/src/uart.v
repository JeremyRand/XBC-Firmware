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

`ifndef _UART
`define _UART

//
// Top UART module -- instantiate only this module
//
module Uart(Addr, DataRd, DataWr, En, Rd, Wr, In, Out, Clk, Reset);

   input  [1:0] Addr;
   output [15:0] DataRd;
   input  [15:0] DataWr;
   input  En;
   input  Rd;
   input  Wr;
   input  In;  // serial data input
   output Out; // serial data output
   input  Clk; 
   input  Reset;

   reg  [7:0] XmitData;
   reg  [15:0] BaudDivisor;
   reg  XmitStart;
   reg  RdSync;
   reg  WrSync;
   reg  EnSync;
   reg  [1:0] AddrSync;
   reg  [15:0] DataWrSync;
   reg  [15:0] DataRd;
   wire [7:0] RecvData;
   wire XmitBusy;
   reg  RecvAck;
   reg  PreAck;
   wire RecvReady;
   wire RecvOverRun;

   UartXmit InstUartXmit(.Clk(Clk), .BaudDivisor(BaudDivisor), .Reset(Reset), 
      .Data(XmitData), .Start(XmitStart), .Busy(XmitBusy), .Out(Out));
   UartRecv InstUartRecv(.Clk(Clk), .BaudDivisor(BaudDivisor), .Reset(Reset), .Data(RecvData), 
      .Ack(RecvAck), .Ready(RecvReady), .OverRun(RecvOverRun), .In(In));
        
   always @(AddrSync or XmitBusy or RecvData or BaudDivisor or RecvReady or RecvOverRun)
      begin
      if (AddrSync==0)                          
         DataRd = {13'b0000000000000, XmitBusy, ~RecvReady, RecvOverRun};
      else if (AddrSync==1)
         DataRd = {8'h00, RecvData};
      else if (AddrSync==2)
         DataRd = BaudDivisor;
      else
         DataRd = 16'hxxxx;
      end

   always @(posedge Clk)
      begin
      RdSync <= Rd;
      WrSync <= Wr;
      EnSync <= En;
      AddrSync <= Addr;
      DataWrSync <= DataWr;
      end

   always @(posedge Clk)
      begin
      if (XmitStart)
         XmitStart <= 1'b0;

      if (WrSync & EnSync)
         begin
         if (AddrSync==1 & ~XmitBusy)
            begin
            XmitData <= DataWrSync[7:0];
            XmitStart <= 1'b1;
            end
         else if (AddrSync==2)
            BaudDivisor <= DataWrSync;
         end
      
                                            
      if (RdSync & EnSync & (AddrSync==1))
         PreAck <= 1'b1;
      else if (PreAck) // end of cycle
         begin
         PreAck <= 1'b0;
         RecvAck <= 1;
         end
      if (RecvAck)
         RecvAck <= 0;

      end
endmodule

module BaudGen (Clk, BaudClk, BaudDivisor, Reset);

   input  Clk;
   output BaudClk;
   input  [15:0] BaudDivisor;
   input  Reset;
   
   reg  [15:0] Count;
   reg  BaudClk;

   always @(posedge Clk or posedge Reset)
      begin
      if (Reset)
         begin
         Count <= 0;
         BaudClk <= 1'b1;
         end
      else
         begin
         if (Count==BaudDivisor)
            begin
            Count <= 0;
            BaudClk <= ~BaudClk;
            end
         else
            Count <= Count + 1;
         end
      end      
endmodule

module UartRecv(Clk, BaudDivisor, Reset, Data, Ack, Ready, OverRun, In);

   input  Clk;
   input  [15:0] BaudDivisor;
   input  Reset;
   output [7:0] Data;
   input  Ack;
   output Ready;
   output OverRun;
   input  In;

   wire  BaudReset;
   reg   [1:0] State;
   reg   InSync;
   reg   PrevBaudClk;
   reg   [2:0] BitCount;
   reg   [7:0] IncomingData;
   reg   [7:0] Data;
   reg   Ready;
   reg   OverRun;

   BaudGen InstBaudGen(.Clk(Clk), .BaudClk(BaudClk), .BaudDivisor(BaudDivisor), .Reset(BaudReset));

   assign BaudReset = (State==0)&~InSync;
   assign EdgeNeg = PrevBaudClk&~BaudClk;

   always @(posedge Clk)
      PrevBaudClk <= BaudClk;

   always @(posedge Clk)
      InSync = In;

   always @(posedge Clk)
      begin
      if (Reset)
         begin
         State <= 0;
         Ready <= 0;
         OverRun <= 0;
         BitCount <= 0;
         end
      else
         begin
         case (State)
         0: // wait for start bit
            begin
            if (~InSync)
               State <= 1;
            BitCount <= 0;
            end

         1: // start bit
            begin
            if (EdgeNeg)
               begin
               if (InSync)
                  State <= 0; // start bit should be low
               else
                  State <= 2; 
               end
            end
         
         2: // receive bits
            begin
            if (EdgeNeg)
               begin
               BitCount <= BitCount + 1;
               IncomingData[BitCount] <= InSync;
               if (BitCount==7)
                  State <= 3;
               end
            end
         3: // stop bit
            begin
            if (EdgeNeg)
               begin
               if (InSync) // stop bit must be high
                  begin
                  if (Ready)
                     OverRun <= 1'b1; // we have overrun the receiver
                  else
                     begin
                     Ready <= 1'b1;
                     Data <= IncomingData;
                     end
                  end
               State <= 0;
               end
            end
         endcase

         if (Ready & Ack)
            Ready <= 1'b0;

         if (OverRun & Ack)
            OverRun <= 1'b0;

         end
      end

endmodule

module UartXmit(Clk, BaudDivisor, Reset, Data, Start, Busy, Out);

   input  Clk;
   input  [15:0] BaudDivisor;
   input  Reset;
   input  [7:0] Data;
   input  Start;
   output Busy;
   output Out;
   
   reg  [1:0] State;
   reg  [3:0] Count;
   reg  [2:0] BitCount;
   reg  Out;   
   wire EdgePos;
   wire BaudClk;
   wire BaudReset;
   reg  PrevBaudClk;

   BaudGen InstBaudGen(.Clk(Clk), .BaudClk(BaudClk), .BaudDivisor(BaudDivisor), .Reset(BaudReset));

   always @(posedge Clk)
      PrevBaudClk <= BaudClk;
                         
   assign Busy = State!=0;
   assign EdgePos = ~PrevBaudClk&BaudClk;
   assign BaudReset = State==0;

   always @(State or BitCount or Data)
      begin
      if (State==1)
         Out = 1'b0; // start bit
      else if (State==2)
         Out = Data[BitCount]; // data bits
      else
         Out = 1'b1;  // idle, stop bit
      end

   always @(posedge Clk)
      begin
      if (Reset)
         begin
         State <= 0;
         BitCount <= 0;
         end
      else
         begin
         case (State)
         0:  // wait for start command
            begin 
            if (Start)
               State <= 1;
            BitCount <= 0;
            end

         1: // send start bit
            begin
            if (EdgePos)
               State <= 2;
            end
           
         2: // send data bits
            begin
            if (EdgePos)
               begin
               BitCount <= BitCount + 1;
               if (BitCount==7)
                  State <= 3;
               end
            end
            
         3: // send stop bit
            begin
            if (EdgePos)
               State <= 0;
            end
                                    
         endcase
         end
      end
endmodule

`endif
