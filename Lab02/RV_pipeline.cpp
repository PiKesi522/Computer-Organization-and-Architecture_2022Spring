#include <bitset>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
#define MemSize 1000  // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

const unsigned long ADD = 0;
const unsigned long SUB = 1;
const unsigned long AND = 2;
const unsigned long OR = 3;
const unsigned long XOR = 4;
const unsigned long ADDI = 5;
const unsigned long BEQ = 6;
const unsigned long JAL = 7;
const unsigned long LD = 8;
const unsigned long SD = 9;

struct IFStruct {
    bitset<32> PC;
    bool nop;
};

struct IF_ID {
    bitset<32> PC;
    bitset<32> Instr;  // 1 for addu, lw, sw, 0 for subu
    bitset<5> Rs1;     // !
    bitset<5> Rs2;     // !
};

struct IDStruct {
    bitset<32> Instr;
    bool nop;
};

struct ID_EX {
    bitset<64> Read_data1;
    bitset<64> Read_data2;
    bitset<64> Imm;
    bitset<5> Rs1;
    bitset<5> Rs2;
    bitset<5> Rd;
    unsigned long ALUop;  // 1 for addu, lw, sw, 0 for subu

    bool is_I_type;
    bool is_J_type = false;
    bool is_R_type;
    bool is_Branch = false;
    bool rd_mem;
    bool wrt_mem;
    bool wrt_enable;

    void clear() {
        this->Read_data1 = bitset<64>(0);
        this->Read_data2 = bitset<64>(0);
        this->Imm = bitset<64>(0);
        this->Rs1 = bitset<5>(0);
        this->Rs2 = bitset<5>(0);
        this->Rd = bitset<5>(0);
        this->is_I_type = false;
        this->is_J_type = false;
        this->is_R_type = false;
        this->is_Branch = false;
        this->wrt_enable = false;
        this->wrt_mem = false;
        this->rd_mem = false;
    }
};

struct EXStruct {
    bitset<64> Read_data1;
    bitset<64> Read_data2;
    bitset<64> Imm;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool is_I_type = false;
    bool rd_mem = false;      // 是否读内存
    bool wrt_mem = false;     // 是否写内存
    unsigned long alu_op;     // 1 for addu, lw, sw, 0 for subu
    bool wrt_enable = false;  // 是否写寄存器
    bool nop;

    // void clear() {
    //     this->Read_data1 = bitset<64>(0);
    //     bitset<64> Read_data2;
    //     bitset<64> Imm;
    //     bitset<5> Rs;
    //     bitset<5> Rt;
    //     bitset<5> Wrt_reg_addr;
    //     bool is_I_type = false;
    //     bool rd_mem = false;      // 是否读内存
    //     bool wrt_mem = false;     // 是否写内存
    //     unsigned long alu_op;     // 1 for addu, lw, sw, 0 for subu
    //     bool wrt_enable = false;  // 是否写寄存器
    //     bool nop;
    // }
};

struct EX_MEM {
    bitset<64> ALUresult;
    bitset<64> Store_data;
    bitset<5> Rs1;
    bitset<5> Rs2;
    bitset<5> Rd;
    bool doBranch = false;  // 比较结果不同（bne），需要跳转
    bool rd_mem = false;
    bool wrt_mem = false;
    bool wrt_enable = false;

    void clear() {
        this->ALUresult = bitset<64>(0);
        this->Store_data = bitset<64>(0);
        this->Rd = bitset<5>(0);
        this->doBranch = false;  // 比较结果不同（bne），需要跳转
        this->rd_mem = false;
        this->wrt_mem = false;
        this->wrt_enable = false;
    }
};

struct MEMStruct {
    bitset<64> ALUresult;
    bitset<64> Store_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool rd_mem = false;
    bool wrt_mem = false;
    bool wrt_enable = false;
    bool nop;
};

struct MEM_WB {
    bitset<64> Wrt_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool wrt_enable;

    void clear() {
        this->Wrt_data = bitset<64>(0);
        this->Wrt_reg_addr = bitset<5>(0);
        this->wrt_enable = false;
    }
};

struct WBStruct {
    bitset<64> Wrt_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool wrt_enable;
    bool nop;
};

struct stateStruct {
    IFStruct IF;
    IDStruct ID;
    EXStruct EX;
    MEMStruct MEM;
    WBStruct WB;
};

/*
 * IF/ID, ID/EX, EX/MEM, MEM/WB寄存器
 */
struct PipelineRegister {
    IF_ID IF_ID_Register;
    ID_EX ID_EX_Register;
    EX_MEM EX_MEM_Register;
    MEM_WB MEM_WB_Register;
};

class RF {
public:
    bitset<64> Reg_data;
    RF() {
        Registers.resize(32);
        Registers[0] = bitset<64>(0);
    }

    void jieDi(){
        this->Registers[0] = bitset<64>("0");
    }

    bitset<64> readRF(bitset<5> Reg_addr) {
        Reg_data = Registers[Reg_addr.to_ulong()];
        return Reg_data;
    }

    void writeRF(bitset<5> Reg_addr, bitset<64> Wrt_reg_data) {
        Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
    }

    void outputRF() {
        ofstream rfout;
        rfout.open("RFresult.txt", std::ios_base::app);
        if (rfout.is_open()) {
            rfout << "State of RF:\t" << endl;
            for (int j = 0; j < 32; j++) {
                rfout << Registers[j] << endl;
            }
        } else
            cout << "Unable to open file";
        rfout.close();
    }

private:
    vector<bitset<64>> Registers;
};

class INSMem {
public:
    bitset<32> Instruction;
    INSMem() {
        IMem.resize(MemSize);
        ifstream imem;
        string line;
        int i = 0;
        imem.open("imem.txt");
        if (imem.is_open()) {
            while (getline(imem, line)) {
                IMem[i] = bitset<8>(line);
                i++;
            }
        } else
            cout << "Unable to open file";
        imem.close();
    }

    bitset<32> readInstr(bitset<32> ReadAddress) {
        string insmem;
        insmem.append(IMem[ReadAddress.to_ulong()].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 1].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 2].to_string());
        insmem.append(IMem[ReadAddress.to_ulong() + 3].to_string());
        Instruction = bitset<32>(insmem);  // read instruction memory
        return Instruction;
    }

private:
    vector<bitset<8>> IMem;
};

class DataMem {
public:
    bitset<64> ReadData;
    DataMem() {
        DMem.resize(MemSize);
        ifstream dmem;
        string line;
        int i = 0;
        dmem.open("dmem.txt");
        if (dmem.is_open()) {
            while (getline(dmem, line)) {
                DMem[i] = bitset<8>(line);
                i++;
            }
        } else
            cout << "Unable to open file";
        dmem.close();
    }

    bitset<64> readDataMem(bitset<32> Address) {
        string datamem;
        datamem.append(DMem[Address.to_ulong()].to_string());
        datamem.append(DMem[Address.to_ulong() + 1].to_string());
        datamem.append(DMem[Address.to_ulong() + 2].to_string());
        datamem.append(DMem[Address.to_ulong() + 3].to_string());
        datamem.append(DMem[Address.to_ulong() + 4].to_string());
        datamem.append(DMem[Address.to_ulong() + 5].to_string());
        datamem.append(DMem[Address.to_ulong() + 6].to_string());
        datamem.append(DMem[Address.to_ulong() + 7].to_string());
        ReadData = bitset<64>(datamem);  // read data memory
        return ReadData;
    }

    void writeDataMem(bitset<32> Address, bitset<64> WriteData) {
        DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0, 8));
        DMem[Address.to_ulong() + 1] = bitset<8>(WriteData.to_string().substr(8, 8));
        DMem[Address.to_ulong() + 2] = bitset<8>(WriteData.to_string().substr(16, 8));
        DMem[Address.to_ulong() + 3] = bitset<8>(WriteData.to_string().substr(24, 8));
        DMem[Address.to_ulong() + 4] = bitset<8>(WriteData.to_string().substr(32, 8));
        DMem[Address.to_ulong() + 5] = bitset<8>(WriteData.to_string().substr(40, 8));
        DMem[Address.to_ulong() + 6] = bitset<8>(WriteData.to_string().substr(48, 8));
        DMem[Address.to_ulong() + 7] = bitset<8>(WriteData.to_string().substr(56, 8));
    }

    void outputDataMem() {
        ofstream dmemout;
        dmemout.open("dmemresult.txt");
        if (dmemout.is_open()) {
            for (int j = 0; j < 1000; j++) {
                dmemout << DMem[j] << endl;
            }
        } else
            cout << "Unable to open file";
        dmemout.close();
    }
    
    void testDataMem(bitset<32> addr){
        cout << "Data : ";
        cout << DMem[addr.to_ulong()];
        cout << DMem[addr.to_ulong() + 1];
        cout << DMem[addr.to_ulong() + 2];
        cout << DMem[addr.to_ulong() + 3];
        cout << DMem[addr.to_ulong() + 4];
        cout << DMem[addr.to_ulong() + 5];
        cout << DMem[addr.to_ulong() + 6];
        cout << DMem[addr.to_ulong() + 7] << endl;
    }

private:
    vector<bitset<8>> DMem;
};

void printState(stateStruct state, int cycle) {
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open()) {
        printstate << "State after executing cycle:\t" << cycle << endl;

        printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
        printstate << "IF.nop:\t" << state.IF.nop << endl;

        printstate << "ID.Instr:\t" << state.ID.Instr << endl;
        printstate << "ID.nop:\t" << state.ID.nop << endl;

        printstate << "EX.Read_data1:\t" << state.EX.Read_data1 << endl;
        printstate << "EX.Read_data2:\t" << state.EX.Read_data2 << endl;
        printstate << "EX.Imm:\t" << state.EX.Imm << endl;
        printstate << "EX.Rs:\t" << state.EX.Rs << endl;
        printstate << "EX.Rt:\t" << state.EX.Rt << endl;
        printstate << "EX.Wrt_reg_addr:\t" << state.EX.Wrt_reg_addr << endl;
        printstate << "EX.is_I_type:\t" << state.EX.is_I_type << endl;
        printstate << "EX.rd_mem:\t" << state.EX.rd_mem << endl;
        printstate << "EX.wrt_mem:\t" << state.EX.wrt_mem << endl;
        printstate << "EX.alu_op:\t" << state.EX.alu_op << endl;
        printstate << "EX.wrt_enable:\t" << state.EX.wrt_enable << endl;
        printstate << "EX.nop:\t" << state.EX.nop << endl;

        printstate << "MEM.ALUresult:\t" << state.MEM.ALUresult << endl;
        printstate << "MEM.Store_data:\t" << state.MEM.Store_data << endl;
        printstate << "MEM.Rs:\t" << state.MEM.Rs << endl;
        printstate << "MEM.Rt:\t" << state.MEM.Rt << endl;
        printstate << "MEM.Wrt_reg_addr:\t" << state.MEM.Wrt_reg_addr << endl;
        printstate << "MEM.rd_mem:\t" << state.MEM.rd_mem << endl;
        printstate << "MEM.wrt_mem:\t" << state.MEM.wrt_mem << endl;
        printstate << "MEM.wrt_enable:\t" << state.MEM.wrt_enable << endl;
        printstate << "MEM.nop:\t" << state.MEM.nop << endl;

        printstate << "WB.Wrt_data:\t" << state.WB.Wrt_data << endl;
        printstate << "WB.Rs:\t" << state.WB.Rs << endl;
        printstate << "WB.Rt:\t" << state.WB.Rt << endl;
        printstate << "WB.Wrt_reg_addr:\t" << state.WB.Wrt_reg_addr << endl;
        printstate << "WB.wrt_enable:\t" << state.WB.wrt_enable << endl;
        printstate << "WB.nop:\t" << state.WB.nop << endl;

        printstate << "===============================================" << endl;
    } else
        cout << "Unable to open file";
    printstate.close();
}

/*
 * ALU: 3位ALUOP对应5种不同的操作
 */
class ALU {
public:
    bitset<64> ALUresult;

    bitset<64> ALUOperation(bitset<3> ALUOP, bitset<64> operand1, bitset<64> operand2) {
        switch (ALUOP.to_ulong()) {
            case ADD:
                ALUresult = bitset<64>(operand1.to_ullong() + operand2.to_ullong());
                break;
            case SUB:
                ALUresult = bitset<64>(operand1.to_ulong() - operand2.to_ulong());
                break;
            case AND:
                ALUresult = bitset<64>(operand1 & operand2);
                break;
            case OR:
                ALUresult = bitset<64>(operand1 | operand2);
                break;
            case XOR:
                ALUresult = bitset<64>(operand1 ^ operand2);
                break;
            default:
                ALUresult = bitset<64>(0);
                break;
        }

        return ALUresult;
    }
};

/*
 * ForwardingUnit: 前递
 */
class ForwardingUnit {
public:
    bitset<2> ForwardA = bitset<2>("00");
    bitset<2> ForwardB = bitset<2>("00");

    void reset() {
        ForwardA = bitset<2>("00");
        ForwardB = bitset<2>("00");
    }

    // TODO 检测冒险
    void detectHazard(PipelineRegister pipelineRegister) {
        reset();
        // EX 冒险
        if (pipelineRegister.EX_MEM_Register.wrt_enable &&
            (pipelineRegister.EX_MEM_Register.Rd != bitset<5>("00000")) &&
            (pipelineRegister.EX_MEM_Register.Rd == pipelineRegister.ID_EX_Register.Rs1)) {
            ForwardA = bitset<2>("10");
            cout << "pipelineRegister.EX_MEM_Register.Rd : " << pipelineRegister.EX_MEM_Register.Rd << endl;
            cout << "pipelineRegister.ID_EX_Register.Rs1 : " << pipelineRegister.ID_EX_Register.Rs1 << endl;
            cout << "Forward A : " << ForwardA << endl;
        }
        if (pipelineRegister.EX_MEM_Register.wrt_enable &&
            (pipelineRegister.EX_MEM_Register.Rd != bitset<5>("00000")) &&
            (pipelineRegister.EX_MEM_Register.Rd == pipelineRegister.ID_EX_Register.Rs2)) {
            ForwardB = bitset<2>("10");
            cout << "pipelineRegister.EX_MEM_Register.Rd : " << pipelineRegister.EX_MEM_Register.Rd << endl;
            cout << "pipelineRegister.ID_EX_Register.Rs2 : " << pipelineRegister.ID_EX_Register.Rs2 << endl;
            cout << "Forward B : " << ForwardB << endl;
        }

        // MEM 冒险
        if (pipelineRegister.MEM_WB_Register.wrt_enable &&
            (pipelineRegister.MEM_WB_Register.Wrt_reg_addr != bitset<5>("00000")) &&
               !(pipelineRegister.EX_MEM_Register.wrt_enable && 
                (pipelineRegister.EX_MEM_Register.Rd != bitset<5>("00000")) && 
                (pipelineRegister.EX_MEM_Register.Rd == pipelineRegister.ID_EX_Register.Rs1)) &&
            (pipelineRegister.MEM_WB_Register.Wrt_reg_addr == pipelineRegister.ID_EX_Register.Rs1)) {
            ForwardA = bitset<2>("01");
            cout << "pipelineRegister.MEM_WB_Register.Wrt_reg_addr : " << pipelineRegister.MEM_WB_Register.Wrt_reg_addr << endl;
            cout << "pipelineRegister.ID_EX_Register.Rs1 : " << pipelineRegister.ID_EX_Register.Rs1 << endl;
            cout << "Forward A : " << ForwardA << endl;
        }
        if (pipelineRegister.MEM_WB_Register.wrt_enable &&
            (pipelineRegister.MEM_WB_Register.Wrt_reg_addr != bitset<5>("00000")) &&
               !(pipelineRegister.EX_MEM_Register.wrt_enable && 
                (pipelineRegister.EX_MEM_Register.Rd != bitset<5>("00000")) && 
                (pipelineRegister.EX_MEM_Register.Rd == pipelineRegister.ID_EX_Register.Rs2)) &&
            (pipelineRegister.MEM_WB_Register.Wrt_reg_addr == pipelineRegister.ID_EX_Register.Rs2)) {
            ForwardB = bitset<2>("01");
            cout << "pipelineRegister.MEM_WB_Register.Wrt_reg_addr : " << pipelineRegister.MEM_WB_Register.Wrt_reg_addr << endl;
            cout << "pipelineRegister.ID_EX_Register.Rs2 : " << pipelineRegister.ID_EX_Register.Rs2 << endl;
            cout << "Forward B : " << ForwardB << endl;
        }
    }

    // 处理冒险
    bool hasHazardForA() {
        return ForwardA != bitset<2>("00");
    }

    bool hasHazardForB() {
        return ForwardB != bitset<2>("00");
    }

    bitset<64> getOperandA(PipelineRegister pipelineRegister) {
        // 此时已发生冒险
        if (ForwardA == bitset<2>("10")) {
            return pipelineRegister.EX_MEM_Register.ALUresult;
        } else {
            return pipelineRegister.MEM_WB_Register.Wrt_data;
        }
    }


    bitset<64> getOperandB(PipelineRegister pipelineRegister) {
        // 此时已发生冒险
        if (ForwardB == bitset<2>("10")) {
            return pipelineRegister.EX_MEM_Register.ALUresult;
        } else {
            return pipelineRegister.MEM_WB_Register.Wrt_data;
        }
    }
};

/*
 * HazardUnit: Ld,Sd冒险控制单元
 */
class HazardUnit {
public:
    bool hazardDetect(PipelineRegister pipelineRegister) {
        return pipelineRegister.ID_EX_Register.Rd == pipelineRegister.IF_ID_Register.Rs1 || pipelineRegister.ID_EX_Register.Rd == pipelineRegister.IF_ID_Register.Rs2;
    }
};

/*
 * Utils
 */
bool isHalt(const bitset<32>& instr) {
    return instr.to_ulong() == 0xffffffff;
}

template <int N>
bitset<N> getBits(const bitset<32>& instr, int start, int end) {
    bitset<N> res;
    for (int i = start; i <= end; ++i) {
        res[i - start] = instr[i];
    }

    return res;
}

template <int N>
bitset<64> generateImm(const bitset<N>& imm) {
    // 将N位立即数扩展成64位的立即数，这里只考虑无符号数，因此高位全用0补全
    bitset<64> imm64;
    for (int i = 0; i < N; ++i) {
        imm64.set(i, imm[i]);
    }
    for (int i = N; i < 64; ++i) {
        imm64.set(i, imm[N - 1]);
    }
    return imm64;
}

bitset<32> getAddress(const bitset<64>& imm) {
    // 将64位立即数转成32为地址，因为是无符号数，直接截取低32位
    bitset<32> addr;
    for (int i = 0; i < 32; ++i) {
        addr.set(i, imm[i]);
    }
    return addr;
}

int main() {
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    PipelineRegister pipelineRegister;
    ALU alu;
    ForwardingUnit forwardingUnit;
    HazardUnit hazardUnit;
    struct stateStruct state {
            0
    };
    state.IF.nop = false;
    state.ID.nop = true;
    state.EX.nop = true;
    state.MEM.nop = true;
    state.WB.nop = true;
    state.EX.alu_op = true;
    int cycle = 0;
    bool halt = false;

    while (1) {
        /* -------------------- 设置此轮周期哪些部件可以操作 -------------------- */
        state.WB.nop = state.MEM.nop || cycle < 4;
        state.MEM.nop = state.EX.nop || cycle < 3;
        state.EX.nop = state.ID.nop || cycle < 2;
        state.ID.nop = state.IF.nop || cycle < 1;
        state.IF.nop = halt;
        if(cycle > 0){
            state.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
        }

        cout << cycle << endl;

        // cout << "pipelineRegister.ID_EX_Register.Rs1 : " << pipelineRegister.ID_EX_Register.Rs1 << endl;
        // cout << "pipelineRegister.ID_EX_Register.Rs2 : " << pipelineRegister.ID_EX_Register.Rs2 << endl;

        /* --------------------- WB stage --------------------- */
        if (!state.WB.nop) {
            state.WB.wrt_enable = pipelineRegister.MEM_WB_Register.wrt_enable;
            state.WB.Wrt_data = pipelineRegister.MEM_WB_Register.Wrt_data;
            state.WB.Rs = pipelineRegister.MEM_WB_Register.Rs;
            state.WB.Rt = pipelineRegister.MEM_WB_Register.Rt;
            state.WB.Wrt_reg_addr = pipelineRegister.MEM_WB_Register.Wrt_reg_addr;

            // 写回目的寄存器
            if (state.WB.wrt_enable) {
                myRF.writeRF(state.WB.Wrt_reg_addr, state.WB.Wrt_data);
                cout << "Target Register : " << state.WB.Wrt_reg_addr.to_ulong() << endl;
                cout << "Data : " << state.WB.Wrt_data.to_ulong() << endl;
            }
        }

        /* --------------------- MEM stage --------------------- */
        if (!state.MEM.nop) {
            state.MEM.ALUresult = pipelineRegister.EX_MEM_Register.ALUresult;
            // cout << "pipelineRegister.EX_MEM_Register.ALUresult : " << pipelineRegister.EX_MEM_Register.ALUresult << endl;
            state.MEM.Store_data = pipelineRegister.EX_MEM_Register.Store_data;
            state.MEM.Rs = pipelineRegister.EX_MEM_Register.Rs1;
            state.MEM.Rt = pipelineRegister.EX_MEM_Register.Rs2;
            state.MEM.Wrt_reg_addr = pipelineRegister.EX_MEM_Register.Rd;
            state.MEM.rd_mem = pipelineRegister.EX_MEM_Register.rd_mem;
            state.MEM.wrt_mem = pipelineRegister.EX_MEM_Register.wrt_mem;
            state.MEM.wrt_enable = pipelineRegister.EX_MEM_Register.wrt_enable;

            pipelineRegister.MEM_WB_Register.clear();

            pipelineRegister.MEM_WB_Register.Rs = state.MEM.Rs;
            pipelineRegister.MEM_WB_Register.Rt = state.MEM.Rt;
            pipelineRegister.MEM_WB_Register.Wrt_data = state.MEM.ALUresult;
            // cout << "pipelineRegister.MEM_WB_Register.Wrt_data : " << pipelineRegister.MEM_WB_Register.Wrt_data << endl;
            if (state.MEM.rd_mem) {
                // ld
                pipelineRegister.MEM_WB_Register.Wrt_data = myDataMem.readDataMem(getAddress(state.MEM.ALUresult));
                cout << "ld Addr : " << getAddress(state.MEM.ALUresult) << endl;
                myDataMem.testDataMem(getAddress(state.MEM.ALUresult));
            }
            if (state.MEM.wrt_mem) {
                // sd
                myDataMem.writeDataMem(getAddress(state.MEM.ALUresult), state.MEM.Store_data);
                // cout << "Already sd to : " << getAddress(state.MEM.ALUresult) << "|||" << state.MEM.Store_data << endl;
                cout << "sd Addr : " << getAddress(state.MEM.ALUresult) << endl;
                myDataMem.testDataMem(getAddress(state.MEM.ALUresult));
            }
            pipelineRegister.MEM_WB_Register.wrt_enable = state.MEM.wrt_enable;
            pipelineRegister.MEM_WB_Register.Wrt_reg_addr = state.MEM.Wrt_reg_addr;
        }

        /* --------------------- EX stage --------------------- */
        if (!state.EX.nop) {
            state.EX.Rs = pipelineRegister.ID_EX_Register.Rs1;
            state.EX.Rt = pipelineRegister.ID_EX_Register.Rs2;
            state.EX.Wrt_reg_addr = pipelineRegister.ID_EX_Register.Rd;
            state.EX.Read_data1 = pipelineRegister.ID_EX_Register.Read_data1;
            state.EX.Read_data2 = pipelineRegister.ID_EX_Register.Read_data2;
            state.EX.Imm = pipelineRegister.ID_EX_Register.Imm;
            state.EX.wrt_enable = pipelineRegister.ID_EX_Register.wrt_enable;
            state.EX.alu_op = pipelineRegister.ID_EX_Register.ALUop;
            state.EX.rd_mem = pipelineRegister.ID_EX_Register.rd_mem;
            state.EX.wrt_mem = pipelineRegister.ID_EX_Register.wrt_mem;
            state.EX.is_I_type = pipelineRegister.ID_EX_Register.is_I_type;

            pipelineRegister.EX_MEM_Register.clear();
            state.EX.Read_data1 = myRF.readRF(pipelineRegister.ID_EX_Register.Rs1);
            state.EX.Read_data2 = myRF.readRF(pipelineRegister.ID_EX_Register.Rs2);


            bitset<64> operandA, operandB;
            forwardingUnit.detectHazard(pipelineRegister);
            if (forwardingUnit.hasHazardForA()) {
                cout << "has hazard A" << endl;
                operandA = forwardingUnit.getOperandA(pipelineRegister);
            } else {
                if (state.EX.rd_mem || state.EX.wrt_mem) {
                    // ld, sd
                    operandA = myRF.readRF(state.EX.Rs);
                } else {
                    // I-type, S-type, R-type
                    operandA = state.EX.Read_data1;
                }
            }

            if (forwardingUnit.hasHazardForB()) {
                cout << "has hazard B" << endl;
                operandB = forwardingUnit.getOperandB(pipelineRegister);
                cout << operandB << endl;
            } else {
                if (state.EX.is_I_type || state.EX.wrt_mem) {
                    // I-type 或 sd
                    operandB = state.EX.Imm;
                } else {
                    // R-type
                    operandB = state.EX.Read_data2;
                }
            }
            
            if (state.EX.rd_mem) {
                cout << "ld" << endl;
                // ld
                pipelineRegister.EX_MEM_Register.rd_mem = true;
                if (hazardUnit.hazardDetect(pipelineRegister)) {
                    cout << "ld hazard" << endl;
                    state.ID.nop = true;
                    state.IF.PC = pipelineRegister.IF_ID_Register.PC;
                }
            }
            if (state.EX.wrt_mem) {
                cout << "sd" << endl;
                // sd
                pipelineRegister.EX_MEM_Register.wrt_mem = true;
                pipelineRegister.EX_MEM_Register.Store_data = operandB;
                if (hazardUnit.hazardDetect(pipelineRegister)) {
                    cout << "sd hazard" << endl;
                    state.ID.nop = true;
                    state.IF.PC = pipelineRegister.IF_ID_Register.PC;
                }
            }

            if (state.EX.wrt_mem || state.EX.rd_mem) {
                // operandA = state.EX.Read_data1;
                operandB = state.EX.Imm;
            }
            // cout << "operandA : " << operandA << endl;
            // cout << "operandB : " << operandB << endl;
            pipelineRegister.EX_MEM_Register.ALUresult = alu.ALUOperation(state.EX.alu_op, operandA, operandB);
            // cout << "pipelineRegister.EX_MEM_Register.ALUresult : " << pipelineRegister.EX_MEM_Register.ALUresult << endl;
            pipelineRegister.EX_MEM_Register.wrt_enable = state.EX.wrt_enable;
            pipelineRegister.EX_MEM_Register.Rs1 = state.EX.Rs;
            pipelineRegister.EX_MEM_Register.Rs2 = state.EX.Rt;
            pipelineRegister.EX_MEM_Register.Rd = state.EX.Wrt_reg_addr;
            
        }

        /* --------------------- ID stage --------------------- */
        if (!state.ID.nop) {
            pipelineRegister.ID_EX_Register.clear();

            state.ID.Instr = pipelineRegister.IF_ID_Register.Instr;
            // 根据指令判断指令类型以及ALUOP
            bitset<7> opcode = getBits<7>(state.ID.Instr, 0, 6);
            bitset<3> funct3 = bitset<3>(0);
            bitset<7> funct7 = bitset<7>(0);

            if (opcode == bitset<7>("0110011")) {
                // R-type
                pipelineRegister.ID_EX_Register.Rs1 = getBits<5>(state.ID.Instr, 15, 19);
                pipelineRegister.ID_EX_Register.Rs2 = getBits<5>(state.ID.Instr, 20, 24);
                // cout << "pipelineRegister.ID_EX_Register.Rs1 : " << pipelineRegister.ID_EX_Register.Rs1 << endl;
                // cout << "pipelineRegister.ID_EX_Register.Rs2 : " << pipelineRegister.ID_EX_Register.Rs2 << endl;

                pipelineRegister.ID_EX_Register.Rd = getBits<5>(state.ID.Instr, 7, 11);
                pipelineRegister.ID_EX_Register.wrt_enable = true;
                funct3 = getBits<3>(state.ID.Instr, 12, 14);
                funct7 = getBits<7>(state.ID.Instr, 25, 31);
                if (funct3 == bitset<3>("000")) {
                    if (funct7 == bitset<7>("0000000")) {
                        cout << "ID op "
                             << "ADD" << endl;
                        pipelineRegister.ID_EX_Register.ALUop = ADD;
                    } else {
                        // sub
                        cout << "ID op "
                             << "SUB" << endl;
                        pipelineRegister.ID_EX_Register.ALUop = SUB;
                    }
                } else if (funct3 == bitset<3>("111")) {
                    // and
                    cout << "ID op "
                            << "AND" << endl;
                    pipelineRegister.ID_EX_Register.ALUop = AND;
                } else if (funct3 == bitset<3>("110")) {
                    // or
                    cout << "ID op "
                            << "OR" << endl;
                    pipelineRegister.ID_EX_Register.ALUop = OR;
                } else {
                    // xor
                    cout << "ID op "
                            << "XOR" << endl;
                    pipelineRegister.ID_EX_Register.ALUop = XOR;
                }
            } else if (opcode == bitset<7>("0010011") || opcode == bitset<7>("0000011")) {
                // I-type
                pipelineRegister.ID_EX_Register.is_I_type = true;
                pipelineRegister.ID_EX_Register.Rs1 = getBits<5>(state.ID.Instr, 15, 19);
                pipelineRegister.ID_EX_Register.Rd = getBits<5>(state.ID.Instr, 7, 11);
                pipelineRegister.ID_EX_Register.Imm = generateImm<12>(getBits<12>(state.ID.Instr, 20, 31));
                pipelineRegister.ID_EX_Register.wrt_enable = true;
                funct3 = getBits<3>(state.ID.Instr, 12, 14);
                cout << "ID op "
                        << "ADDI" << endl;
                pipelineRegister.ID_EX_Register.ALUop = ADD;
                if (funct3 == bitset<3>("011")) {
                    // ld
                    cout << "ID op "
                            << "LD" << endl;
                    pipelineRegister.ID_EX_Register.rd_mem = true;
                }
            } else if (opcode == bitset<7>("0100011")) {
                // S-type
                // sd
                // RS1 用于计算地址偏移
                pipelineRegister.ID_EX_Register.Rs1 = getBits<5>(state.ID.Instr, 15, 19);
                // RS2 用于得到源地址
                pipelineRegister.ID_EX_Register.Rs2 = getBits<5>(state.ID.Instr, 20, 24);
                bitset<5> imm1 = getBits<5>(state.ID.Instr, 7, 11);
                bitset<7> imm2 = getBits<7>(state.ID.Instr, 25, 31);
                bitset<12> imm;
                cout << "ID op "
                        << "SD" << endl;
                for (int i = 0; i <= 4; ++i) {
                    imm[i] = imm1[i];
                }
                for (int i = 0; i <= 11; ++i) {
                    imm[i + 5] = imm2[i];
                }
                pipelineRegister.ID_EX_Register.Imm = generateImm<12>(imm);
                cout << "Imm:" << pipelineRegister.ID_EX_Register.Imm << endl;
                pipelineRegister.ID_EX_Register.wrt_enable = false;
                pipelineRegister.ID_EX_Register.wrt_mem = true;
                pipelineRegister.ID_EX_Register.ALUop = ADD;
            } else if (opcode == bitset<7>("1100111")) {
                // SB-type
                // beq

                pipelineRegister.ID_EX_Register.Rs1 = getBits<5>(state.ID.Instr, 15, 19);
                pipelineRegister.ID_EX_Register.Rs2 = getBits<5>(state.ID.Instr, 20, 24);
                bitset<4> imm1 = getBits<4>(state.ID.Instr, 8, 11);
                bitset<6> imm2 = getBits<6>(state.ID.Instr, 25, 30);
                bitset<13> imm;
                cout << "ID op "
                        << "BEQ" << endl;
                imm[0] = false;  // SB-type最低位一定是0，保证是偶数
                for (int i = 1; i <= 4; ++i) {
                    imm[i] = imm1[i - 1];
                }
                for (int i = 5; i <= 10; ++i) {
                    imm[i] = imm2[i - 5];
                }
                imm[11] = state.ID.Instr[7];
                imm[12] = state.ID.Instr[31];
                cout << "beq imm : " << imm << endl;
                pipelineRegister.ID_EX_Register.Imm = generateImm<13>(imm);
                // cout << "pipelineRegister.ID_EX_Register.Imm : " << pipelineRegister.ID_EX_Register.Imm << endl;
                pipelineRegister.ID_EX_Register.ALUop = ADD;
                pipelineRegister.ID_EX_Register.wrt_enable = false;
                // TODO 在ID阶段就做分支跳转的判断
                pipelineRegister.ID_EX_Register.is_Branch = true;
            } else {
                // UJ-type
                // jal
                // TODO 在ID阶段就做分支跳转的判断
                pipelineRegister.ID_EX_Register.Rd = getBits<5>(state.ID.Instr, 7, 11);
                bitset<10> imm1 = getBits<10>(state.ID.Instr, 21, 30);
                bitset<8> imm2 = getBits<8>(state.ID.Instr, 12, 19);
                bitset<21> imm;
                cout << "ID op "
                        << "JAL" << endl;
                imm[0] = false;  // UJ-type最低位一定是0，保证是偶数
                for (int i = 1; i <= 10; ++i) {
                    imm[i] = imm1[i - 1];
                }
                imm[11] = state.ID.Instr[20];
                for (int i = 12; i <= 19; ++i) {
                    imm[i] = imm2[i - 12];
                }
                imm[20] = state.ID.Instr[31];
                cout << "jal imm : " << imm << endl;
                pipelineRegister.ID_EX_Register.Imm = generateImm<21>(imm);
                // cout << "pipelineRegister.ID_EX_Register.Imm : " << pipelineRegister.ID_EX_Register.Imm << endl;
                pipelineRegister.ID_EX_Register.wrt_enable = true;
                pipelineRegister.ID_EX_Register.is_J_type = true;
            }

            // TODO Read_data
            pipelineRegister.ID_EX_Register.Read_data1 = myRF.readRF(pipelineRegister.ID_EX_Register.Rs1);
            pipelineRegister.ID_EX_Register.Read_data2 = myRF.readRF(pipelineRegister.ID_EX_Register.Rs2);

            // TODO 判断两个分支指令是否发生跳转
            if (pipelineRegister.ID_EX_Register.is_Branch){
                if(pipelineRegister.ID_EX_Register.Read_data1 != pipelineRegister.ID_EX_Register.Read_data2){
                    // cout << "pipelineRegister.ID_EX_Register.Read_data1 : " << pipelineRegister.ID_EX_Register.Read_data1 << endl;
                    // cout << "pipelineRegister.ID_EX_Register.Read_data2 : " << pipelineRegister.ID_EX_Register.Read_data2 << endl;
                    // cout << "pipelineRegister.IF_ID_Register.PC.to_ulong() : " << pipelineRegister.IF_ID_Register.PC.to_ullong() << endl;
                    // cout << "pipelineRegister.ID_EX_Register.Imm.to_ulong() : " << pipelineRegister.ID_EX_Register.Imm.to_ullong() << endl;
                    state.IF.PC = bitset<32>(pipelineRegister.IF_ID_Register.PC.to_ulong() + pipelineRegister.ID_EX_Register.Imm.to_ulong());
                    cout << "branch to " << state.IF.PC.to_ulong() << endl;
                }else{
                    cout << "branch fail" << endl;
                }
                pipelineRegister.ID_EX_Register.clear();
                cout << "pipelineRegister.ID_EX_Register.Imm : " << pipelineRegister.ID_EX_Register.Imm << endl;
                // cout << state.
                state.ID.Instr = bitset<32>(0);
            }

            if (pipelineRegister.ID_EX_Register.is_J_type) {
                // cout << "pipelineRegister.IF_ID_Register.PC.to_ulong() : " << pipelineRegister.IF_ID_Register.PC.to_ullong() << endl;
                // cout << "pipelineRegister.ID_EX_Register.Imm.to_ulong() : " << pipelineRegister.ID_EX_Register.Imm.to_ullong() << endl;
                state.IF.PC = bitset<32>(pipelineRegister.IF_ID_Register.PC.to_ullong() + pipelineRegister.ID_EX_Register.Imm.to_ullong());
                cout << "jump to " << state.IF.PC.to_ulong() << endl;
                pipelineRegister.ID_EX_Register.clear();
                state.ID.Instr = bitset<32>(0);
            }
        }

        /* --------------------- IF stage --------------------- */
        if (!state.IF.nop) {
            // 取指
            // 存入IF/ID寄存器
            cout << "PC : " << state.IF.PC << endl;
            pipelineRegister.IF_ID_Register.Instr = myInsMem.readInstr(state.IF.PC);
            pipelineRegister.IF_ID_Register.PC = state.IF.PC;
            pipelineRegister.IF_ID_Register.Rs1 = getBits<5>(state.ID.Instr, 15, 19);
            pipelineRegister.IF_ID_Register.Rs2 = getBits<5>(state.ID.Instr, 20, 24);

            // 判断是不是最后的指令
            halt = isHalt(pipelineRegister.IF_ID_Register.Instr);

            // TODO 更新PC
        }

        /* --------------------- Leave stage --------------------- */
        if ((state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop) || cycle == 70){
            break;
        }

        printState(state, cycle);
        myRF.jieDi();
        // printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ...
        cout << "==========================================" << endl;
        cycle += 1;
        // state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */
    }

    myRF.outputRF();            // dump RF;
    myDataMem.outputDataMem();  // dump data mem

    return 0;
}
