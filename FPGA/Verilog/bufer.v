`include "define.v"

module bufer(
input wire CLK1,
input wire CLK2,
input wire RESET,
input wire start_write,
input wire BUFER_CHANGE,
input wire BUFER_IN_EN,
input wire [`FSMC_WIDTH-1:0] DATA_IN,
input wire [`DATA_READ_RAZR-1:0]NUMB_BYTE_IN,
output [`FSMC_WIDTH-1:0] DATA_OUT,
input wire [`DATA_READ_RAZR-1:0]NUMB_BYTE_OUT

);

reg [`FSMC_WIDTH-1:0] bufer1[`LENTH_BUFER-1:0];
reg [`FSMC_WIDTH-1:0] bufer2[`LENTH_BUFER-1:0];



reg [`SPI_WIDHT-1:0] out=8'b0;
always @(negedge CLK1 or posedge RESET )
begin
    if (RESET)
    begin
        
    end
    else
	 begin
			if( BUFER_IN_EN==1)
        begin
			  if (BUFER_CHANGE==1)
			  begin
					bufer1[NUMB_BYTE_IN]<=DATA_IN;
			  end
			  else
			  begin
					bufer2[NUMB_BYTE_IN]<=DATA_IN;
			  end
         end
			
		
    end
end

always @(posedge CLK2  or posedge RESET)
begin
    if (RESET)
    begin
       

    end
    else
	 begin
		  if (BUFER_CHANGE==0)
        begin
		    out<=bufer1[NUMB_BYTE_OUT];
		  end
        else
        begin
		    out<=bufer2[NUMB_BYTE_OUT];
		  end
       
	 end
end

assign DATA_OUT=out;
endmodule 

