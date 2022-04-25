#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    // PCplus4是当前指令PC+4的临时值，如果没有跳转，那么下次PC=PC+4；反之，PC=offset。使得PC在下次被ID使用的时候不会变化
    bitset<32>  PCplus4;
    bool        nop;
};

struct IFIDStruct {
    bitset<32>  PC;
    bitset<32>  Instr;   //1 for addu, lw, sw, 0 for subu 
    bool        nop;  
};

struct IDEXStruct {
    bitset<64>  Read_data1;
    bitset<64>  Read_data2;
    bitset<64>  Imm;
    bitset<32>  PC;
    bitset<5>   Rs1;
    bitset<5>   Rs2;
    bitset<5>   Rd;
    bitset<4>   ALUop;     //1 for addu, lw, sw, 0 for subu 
    bool        is_I_type;
    bool        is_J_type;
    bool        is_R_type;
    bool        is_Branch;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;
    bool        nop;  
};

struct EXMEMStruct {
    bitset<64>  ALUresult;
    bitset<64>  Store_data;
    bitset<32>  new_Addr;   // 通过PC + 4 + offset得到的新的地址
    bitset<5>   Rs1;
    bitset<5>   Rs2;
    bitset<5>   Rd;
    bool        doBranch;   // 比较结果不同（bne），需要跳转
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
};

struct WBStruct {
    bitset<64>  Wrt_data;
    bitset<5>   Rs1;
    bitset<5>   Rs2;     
    bitset<5>   Rd;
    bool        wrt_enable;
    bool        nop;     
};

struct stateStruct {
    IFStruct        IF;
    IFIDStruct      IFID;
    IDEXStruct      IDEX;
    EXMEMStruct     EXMEM;
    WBStruct        WB;
};

class RF
{
    public: 
        bitset<64> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<64> (0);  
        }
	
        bitset<64> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<64> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<64> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8>> IMem;     
};
      
class DataMem    
{
    public:
        bitset<64> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<64> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            datamem.append(DMem[Address.to_ulong()+4].to_string());
            datamem.append(DMem[Address.to_ulong()+5].to_string());
            datamem.append(DMem[Address.to_ulong()+6].to_string());
            datamem.append(DMem[Address.to_ulong()+7].to_string());            
            ReadData = bitset<64>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<64> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
	        DMem[Address.to_ulong()+4] = bitset<8>(WriteData.to_string().substr(32,8));
            DMem[Address.to_ulong()+5] = bitset<8>(WriteData.to_string().substr(40,8));
            DMem[Address.to_ulong()+6] = bitset<8>(WriteData.to_string().substr(48,8));
            DMem[Address.to_ulong()+7] = bitset<8>(WriteData.to_string().substr(56,8));   
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.IFID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.IFID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.IDEX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.IDEX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.IDEX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.IDEX.Rs1<<endl;
        printstate<<"EX.Rt:\t"<<state.IDEX.Rs2<<endl;
        printstate<<"EX.Rd:\t"<<state.IDEX.Rd<<endl;
        printstate<<"EX.is_I_type:\t"<<state.IDEX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.IDEX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.IDEX.wrt_mem<<endl;        
        printstate<<"EX.ALUop:\t"<<state.IDEX.ALUop<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.IDEX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.IDEX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.EXMEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.EXMEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.EXMEM.Rs1<<endl;
        printstate<<"MEM.Rt:\t"<<state.EXMEM.Rs2<<endl;   
        printstate<<"MEM.Rd:\t"<<state.EXMEM.Rd<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.EXMEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.EXMEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.EXMEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.EXMEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs1<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rs2<<endl;        
        printstate<<"WB.Rd:\t"<<state.WB.Rd<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}
 
bool isHalt(bitset<32>& const PC){
    return PC.to_ulong() == 0xffffffff;
}

int main()
{
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    struct stateStruct state{0};
    struct stateStruct newState{0};
    state.IF.nop = false;
    state.IFID.nop = true;
    state.IDEX.nop = true;
    state.EXMEM.nop = true;
    state.WB.nop = true;
    state.IDEX.ALUop = true;
    int cycle = 0;

    while (1) {

        /* --------------------- WB stage --------------------- */
        if(state.WB.nop && state.EXMEM.nop){
            state.WB.nop = false;
        }else{
            if (state.EXMEM.doBranch){
                cout << "Success Jmp" << endl;
                state.IF.PCplus4 = state.EXMEM.new_Addr;
            }else if(state.EXMEM.rd_mem){

            }else if(state.EXMEM.wrt_mem){

            }else if(state.EXMEM.wrt_enable){
                
            }else{
                cout << "Do nothing" << endl;
            }
        }



        /* --------------------- EXMEM stage --------------------- */
        /*
        struct EXMEMStruct {
            bitset<64>  ALUresult;
            bitset<64>  Store_data; // 要被存储回Dmem的数据，根据wrt_enable从ALUresult赋值
            bitset<32>  new_Addr;   // 通过PC + 4 + offset得到的新的地址
            bitset<5>   Rs1;
            bitset<5>   Rs2;
            bitset<5>   Rd;
            bool        doBranch;   // 比较结果不同（bne）或者是jal，需要跳转
            bool        rd_mem;     // =isLoad
            bool        wrt_mem;    // =isStore
            bool        wrt_enable; // =wrtEnable
            bool        nop;
        };
        EXMEM阶段要做的事情：
            1.通过多选1，来决定ALU的Rs1来自于：(Rs2同理)
                上一步得到的Rs1
                相邻步得到的EX.Rd
                次相邻的道德MEM.Rd
            2.通过IDEX的ALUop来计算此步骤的ALUresult（根据ALUop，选择Rs2或imm）（立即数左移1位，和PC+4相加，在beq,jal情况下进行跳转）
            3.如果wrt_enable，那么将ALUresult赋予Store_data
        */
        if(state.EXMEM.nop && state.IDEX.nop){
            state.EXMEM.nop = false;
        }else{
            // Forwarding Unit
            // 1.通过多选1，来决定ALU的Rs1来自于：(Rs2同理)
            state.EXMEM.Rs1 = ForwardingRs1();
            state.EXMEM.Rs2 = ForwardingRs2();
            state.EXMEM.Rd = state.IDEX.Rd;
            state.EXMEM.wrt_enable = state.IDEX.wrt_enable;
            state.EXMEM.wrt_mem = state.IDEX.wrt_mem;
            state.EXMEM.rd_mem = state.IDEX.rd_mem;
            state.EXMEM.doBranch = false;

            // 2.通过IDEX的ALUop来计算此步骤的ALUresult
            switch (state.IDEX.ALUop.to_ulong()){
                case 0:
                    // 0000 add
                    state.EXMEM.ALUresult = bitset<64>(state.IDEX.Read_data1.to_ullong() + state.IDEX.Read_data2.to_ullong());
                    break;
                case 1:
                    // 0001 addi
                    state.EXMEM.ALUresult = bitset<64>(state.IDEX.Read_data1.to_ullong() + state.IDEX.Imm.to_ullong());
                    break;
                case 2:
                    // 0010 sub
                    state.EXMEM.ALUresult = bitset<64>(state.IDEX.Read_data1.to_ullong() - state.IDEX.Read_data2.to_ullong());
                    break;
                case 4:
                    // 0100 and
                    state.EXMEM.ALUresult = bitset<64>(state.IDEX.Read_data1.to_ullong() & state.IDEX.Read_data2.to_ullong());
                    break;
                case 6:
                    // 0110 or
                    state.EXMEM.ALUresult = bitset<64>(state.IDEX.Read_data1.to_ullong() | state.IDEX.Read_data2.to_ullong());
                    break;
                case 8:
                    // 1000 xor
                    state.EXMEM.ALUresult = bitset<64>(state.IDEX.Read_data1.to_ullong() ^ state.IDEX.Read_data2.to_ullong());
                    break;
                case 10: // sw/lw 和 addi都是将Rs1和imm进行相加，所以其实可以合并，但是为了便于修改不合并
                    // 1010 sw/lw
                    state.EXMEM.ALUresult = bitset<64>(state.IDEX.Read_data1.to_ullong() + state.IDEX.Imm.to_ullong());
                    break;
                case 12:
                    // 1100 jal
                    state.EXMEM.doBranch = true;
                    state.EXMEM.new_Addr = bitset<32>(state.IDEX.Imm.to_ullong() + state.IDEX.PC.to_ullong());
                    break;
                case 14:
                    // 1110 bne
                    state.EXMEM.doBranch = (state.IDEX.Read_data1 != state.IDEX.Read_data2);
                    state.EXMEM.new_Addr = bitset<32>(state.IDEX.Imm.to_ullong() + state.IDEX.PC.to_ullong());
                    break;
                default:
                    cout << "ERROR" << endl;
                    break;
            }
            
            // 3.如果wrt_enable，那么将ALUresult赋予Store_data
            if(state.EXMEM.wrt_enable){
                state.EXMEM.Store_data = state.EXMEM.ALUresult;
            }
        }






        /* --------------------- IDEX stage --------------------- */
        /*
        struct IDEXStruct {
            bitset<64>  Read_data1;
            bitset<64>  Read_data2;
            bitset<64>  Imm;
            bitset<32>  PC;
            bitset<5>   Rs1;
            bitset<5>   Rs2;
            bitset<5>   Rd;
            bitset<4>   ALUop;
            bool        is_I_type;  // 用于判别addi
            bool        is_J_type;
            bool        is_R_type;
            bool        is_Branch;
            bool        rd_mem;     // set for lw
            bool        wrt_mem;    // set for sw
            bool        wrt_enable; // 是否写回寄存器
            bool        nop;
        };
        IDEX阶段要做的事情：
            1.记录本指令的PC+4
            2.判断指令类型(lw,sw,J,R...)
            2.获取Rs1,Rs2,Rd
            4.根据不同类型指令获取Imm
            5.确定ALUop
        */
        if(state.IDEX.nop && state.IFID.nop){
            state.IDEX.nop = false;
        }else{
            // 1.记录本指令的PC+4
            state.IDEX.PC = state.IFID.PC;

            // 2.判断指令类型
            {
                state.IDEX.rd_mem = (state.IFID.Instr.to_string().substr(25, 7) == string("0000011"));
                state.IDEX.wrt_mem = (state.IFID.Instr.to_string().substr(25, 7) == string("0100011"));
                state.IDEX.is_J_type = state.IFID.Instr.to_string().substr(25, 7) == string("1101111");
                state.IDEX.is_R_type = state.IFID.Instr.to_string().substr(25, 7) == string("0110011");
                state.IDEX.is_Branch = state.IFID.Instr.to_string().substr(25, 7) == string("1100111");
                state.IDEX.is_I_type = (state.IFID.Instr.to_string().substr(25, 5) == string("00100"));
                state.IDEX.wrt_enable = !(state.IDEX.wrt_mem || state.IDEX.is_Branch);
            }

            // 3.根据指令类型分析源寄存器并读取，目的寄存器
            {
                state.IDEX.Rs1 = (state.IDEX.is_J_type) ? bitset<5>(string("00000")) : bitset<5>(state.IFID.Instr.to_string().substr(12, 5));
                state.IDEX.Read_data1 = myRF.readRF(state.IDEX.Rs1);
                state.IDEX.Rs2 = (state.IDEX.is_I_type || state.IDEX.is_J_type || state.IDEX.rd_mem) ? bitset<5>(string("00000")) : bitset<5>(state.IFID.Instr.to_string().substr(7, 5));
                state.IDEX.Read_data2 = myRF.readRF(state.IDEX.Rs2);
                state.IDEX.Rd = (state.IDEX.is_I_type || state.IDEX.is_R_type || state.IDEX.is_J_type || state.IDEX.rd_mem) ? bitset<5>(state.IFID.Instr.to_string().substr(20, 5)) : bitset<5>(string("00000"));
            }

            // 4.*****************立即数计算，存疑***********************  
            {     
                if (state.IDEX.rd_mem || state.IDEX.is_I_type){
                    state.IDEX.Imm = bitset<64>(state.IFID.Instr.to_string().substr(0, 12));
                    if (state.IDEX.Imm[20] == true)
                        state.IDEX.Imm = bitset<64>(string(52, '1') + state.IDEX.Imm.to_string().substr(20, 12));
                }else if (state.IDEX.wrt_mem == 1){
                    state.IDEX.Imm = bitset<64>(state.IFID.Instr.to_string().substr(0, 7) + state.IFID.Instr.to_string().substr(20, 5));
                    if (state.IDEX.Imm[20] == true)
                        state.IDEX.Imm = bitset<64>(string(52, '1') + state.IDEX.Imm.to_string().substr(20, 12));
                }else if (state.IDEX.is_J_type){
                    if (state.IFID.Instr[31] == true)
                        state.IDEX.Imm = bitset<64>(string(11, '1') + state.IFID.Instr.to_string().substr(0, 1) + state.IFID.Instr.to_string().substr(12, 8) 
                                                    + state.IFID.Instr.to_string().substr(11, 1) + state.IFID.Instr.to_string().substr(1, 10) + string("0"));
                    else
                        state.IDEX.Imm = bitset<64>(string(11, '0') + state.IFID.Instr.to_string().substr(0, 1) + state.IFID.Instr.to_string().substr(12, 8) 
                                                    + state.IFID.Instr.to_string().substr(11, 1) + state.IFID.Instr.to_string().substr(1, 10) + string("0"));
                }else if(state.IDEX.is_Branch){
                    if (state.IFID.Instr[31] == true)
                        state.IDEX.Imm = bitset<64>(string(19, '1') + state.IFID.Instr.to_string().substr(0, 1) + state.IFID.Instr.to_string().substr(24, 1) 
                                                    + state.IFID.Instr.to_string().substr(1, 6) + state.IFID.Instr.to_string().substr(20, 4) + string("0"));
                    else
                        state.IDEX.Imm = bitset<64>(string(19, '0') + state.IFID.Instr.to_string().substr(0, 1) + state.IFID.Instr.to_string().substr(24, 1) 
                                                    + state.IFID.Instr.to_string().substr(1, 6) + state.IFID.Instr.to_string().substr(20, 4) + string("0"));
                }else{
                    state.IDEX.Imm = bitset<64>(0);
                }
            }
            //   *****************立即数计算，存疑***********************
        
            // 5.确定ALUop
            {
                if (state.IDEX.is_R_type){
                    if (state.IFID.Instr.to_string().substr(17, 3) == string("000")){
                        if (state.IFID.Instr.to_string().substr(0, 7) == string("0000000")){
                            state.IDEX.ALUop = bitset<4>("0000"); // add
                        }else if (state.IFID.Instr.to_string().substr(0, 7) == string("0100000")){
                            state.IDEX.ALUop = bitset<4>("0010"); // sub
                        }
                    }else if (state.IFID.Instr.to_string().substr(17, 3) == string("111")){
                        state.IDEX.ALUop = bitset<4>("0100"); // and
                    }else if (state.IFID.Instr.to_string().substr(17, 3) == string("110")){
                        state.IDEX.ALUop = bitset<4>("0110"); // or
                    }else if (state.IFID.Instr.to_string().substr(17, 3) == string("100")){
                        state.IDEX.ALUop = bitset<4>("1000"); // xor
                    }
                }else if (state.IDEX.wrt_mem == 1 || state.IDEX.rd_mem == 1){
                    state.IDEX.ALUop = bitset<4>("1010"); // sw or lw
                }else if (state.IDEX.is_J_type){
                    state.IDEX.ALUop = bitset<4>("1100"); // jal
                }else if (state.IDEX.is_I_type){
                    state.IDEX.ALUop = bitset<4>("0001"); // addi，0000add的同分异构体
                }else{
                    state.IDEX.ALUop = bitset<4>("1110"); // beq
                }
            }
        }








        /* --------------------- IFID stage --------------------- */
        /*  
        struct IFIDStruct {
            bitset<32>  PC;
            bitset<32>  Instr;
            bool        nop;  
        };
        IFID阶段需要做的事情：
            1.记录此步骤的PC+4的结果
            2.将PC所对应的指令从Imem中读取
        */
        if(state.IFID.nop && state.IF.nop){
            state.IFID.nop = false;
        }else{
            // 1.记录此步骤的PC+4的结果
            state.IFID.PC = state.IF.PCplus4;
            
            // 2.将PC所对应的指令从Imem中读取
            state.IFID.Instr = myInsMem.readInstr(state.IF.PC);
        }






        /* --------------------- IF stage --------------------- */
        state.IF.PC = state.IF.PCplus4;
        if(isHalt(state.IF.PC)){
            // halt为最终的停止指令，如果IF为nop，那么程序执行完毕
            state.IF.nop = true;
        }else{
            state.IF.nop = false;
            state.IF.PCplus4 = bitset<32>(state.IF.PC.to_ulong() + 4);
        }





        /* --------------------- Stall unit--------------------- */







        if (state.IF.nop && state.IFID.nop && state.IDEX.nop && state.EXMEM.nop && state.WB.nop)
            break;

        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 

        cycle += 1;
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */

    }

    myRF.outputRF(); // dump RF;	
    myDataMem.outputDataMem(); // dump data mem 

    return 0;
}
