<center>
    <h2>
        计算机组成与设计 Lab2
    </h2>
</center>


<center>
    <h4>
       	10192100571 俞辰杰
    </h4>
</center>

### 实验目的

------

​		**本测试压缩包包括三个测试样例，其中Imem.txt是本样例，既存在RAW冒险，也存在分支跳转指令进行提前预测。**

​		**其余两个测试样例Imem1.txt和Imem2.txt分别是只存在RAW冒险的测试样例和不存在冒险的测试样例。**

​		**如果需要测试自己的代码，请按需要更改imem.txt和dmem.txt。**

------

​		在此实验中，运用C++来模拟RISCV的指令级别的流水线下的单周期循环。在此次实验，为了模拟并行CPU，所采用的代码编写方法仍旧是顺序编写，但是是逆序执行，也即先执行同一时钟周期后半部分的代码，再逐渐往前执行。

​		为了有效模拟Risc-V流水线级的实现，对于五个阶段"State Struct"，四个流水线寄存器"Pipeline Register"都有实现。

​		为了优化流水线结构，需要进行Forwarding Unit控制数据冒险以及Hazard Unit控制结构冒险。

​		在本次实验中，还优化了beq和jal的判断方式，在ID阶段就可以判断是否要进行跳转。同时在本次实验中假定了beq和jal不会遇到数据冒险，所以实现方式不需要额外的Unit。

​		本次实验需要实现的功能如下：

| **Name** | **Format Type** | **Opcode** (Binary) | **Func** **3(Binary)** | **Func** **7(Binary)** |
| -------- | --------------- | ------------------- | ---------------------- | ---------------------- |
| **add**  | R-Type          | 0110011             | 000                    | 0000000                |
| **sub**  | R-Type          | 0110011             | 000                    | 0100000                |
| **addi** | I-Type          | 0010011             | 000                    | --                     |
| **and**  | R-Type          | 0110011             | 111                    | 0000000                |
| **or**   | R-Type          | 0110011             | 110                    | 0000000                |
| **xor**  | R-Type          | 0110011             | 100                    | 0000000                |
| **beq**  | SB-Type         | 1100011             | 000                    | --                     |
| **jal**  | UJ-Type         | 1101111             | --                     | --                     |
| **ld**   | I-Type          | 0000011             | 011                    | --                     |
| **sd**   | S-Type          | 0100011             | 011                    | --                     |

<div STYLE="page-break-after: always;"></div>

### 实验步骤

------

- **使用参考代码，可以直接运行RISC-V.cpp，运行代码如下所示：**

​		样例程序的C语言代码，RISCV汇编代码，以及对于的指令集的机器码存放于此

~~~C
int A[4] = {0,0,0,10,0};
int cnt = 10;
int small = 0;
int i = 0;
while(true){
    cnt -= 1;
    small += 1;
    if(A[i] != 0){
        break;
    }
    A[i] = i++;
}
A[i] = cnt;
~~~

​		此外，寄存器初始化和数据内存在代码中进行设定

~~~compile
x1  : 1
x10 : A[]
X12 : cnt
X13 : small
X14 : i
X28	: temp_arg
X29	: temp_addr
~~~

​	**对应的riscv指令及32位指令码**

~~~c
/*-----初始化-----*/
00	addi X1, X0, 1		// X1 = 1
000000000001  00000 000 00001 0010011
04	addi X12, X0, 10	// int cnt = 10
000000001010  00000 000 01100 0010011
08	sd X12, 24(X10)		// A[3] = 10		// Dmem[(3 + 1) * 8 = 32] = 11000
0000000 01100 01010 111 11000 0100011		// 24 = 11000
While_Loop:
12	add X29, X14, X14
0000000 01110 01110 000 11101 0110011
16	add X29, X29, X29
0000000 11101 11101 000 11101 0110011
20	add X29, X29, X29	// X29 = 8 * i
0000000 11101 11101 000 11101 0110011
24	add X29, X29, X10	// X29 = &A[i]
0000000 11101 01010 000 11101 0110011
28	ld X28, 0(X29)		// X28 = A[i]
000000000000  11101 011 11100 0000011
32	sub X12, X12, X1	// cnt -= 1
0100000 00001 01100 000 01100 0110011
36  addi x13, x13, 1	// small ++
000000000001  01101 000 01101 0010011
BNE:
40	bne x28, x0, EXIT	// if(A[i] != 0)	// PC += 56 - 40 = 16
0 000000 11100 00000 000 1000 0 1100111		// 20 = [0]1000/0
44	sd X14, 0(X29)		// A[i] = i
0000000 01110 11101 011 00000 0100011
48	addi X14, X14, 1	// i ++;
000000000001  01110 000 01110 0010011
52	jal x0, While_LOOP						// Pc + offset = 12, offset = -40
1 1111101100 1 11111111 00000 1101111		// 40 = [0]10100/0; -40 = [1]01100/0 
EXIT:
56	sd X12, 0(X29)		// A[i] = cnt
0000000 01100 11101 011 00000 0100011
60	HALT
1111111 11111 11111 111 11111 1111111
~~~

~~~bina
00000000000100000000000010010011
00000000101000000000011000010011
00000000110001010111110000100011
00000000111001110000111010110011
00000001110111101000111010110011
00000001110111101000111010110011
00000001110101010000111010110011
00000000000011101011111000000011
01000000000101100000011000110011
00000000000101101000011010010011
00000001110000000000100001100111
00000000111011101011000000100011
00000000000101110000011100010011
11111101100111111111000001101111	
00000000110011101011000000100011
11111111111111111111111111111111
~~~

<div STYLE="page-break-after: always;"></div>

### 实验结果

------

**按照imem.txt的代码，得到的结果应该为：**

- **详细信息在stateresult.txt中可见**

- **在寄存器堆Register File中：**
  - 【X12】cnt =  6
  - 【X13】small =  4
  - 【X14】i =  3

![image-20220515161651887](C:\Users\86008\AppData\Roaming\Typora\typora-user-images\image-20220515161651887.png)



- **在内存Data Memory中：**
  - DMem[15] = 0000 0001
  - DMem[23] = 0000 0010
  - DMem[31] = 0000 0110

<img src="C:\Users\86008\AppData\Roaming\Typora\typora-user-images\image-20220515161834527.png" alt="image-20220515161834527" style="zoom:67%;" />

<div STYLE="page-break-after: always;"></div>

### 实验代码

------

- **五个阶段和四个流水线寄存器的结构体**

~~~C
struct IFStruct {
    bitset<32> PC;
    bool nop;
};

struct IF_ID {
    bitset<32> PC;
    bitset<32> Instr;
    bitset<5> Rs1；
    bitset<5> Rs2;
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
    unsigned long ALUop;

    bool is_I_type;
    bool is_J_type = false;
    bool is_R_type;
    bool is_Branch = false;
    bool rd_mem;
    bool wrt_mem;
    bool wrt_enable;

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
    unsigned long alu_op; 
    bool wrt_enable = false;  // 是否写寄存器
    bool nop;
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
};

struct WBStruct {
    bitset<64> Wrt_data;
    bitset<5> Rs;
    bitset<5> Rt;
    bitset<5> Wrt_reg_addr;
    bool wrt_enable;
    bool nop;
};
~~~



- **一般数据冒险控制单元 Forwarding Unit**

~~~C++
class ForwardingUnit {
public:
    bitset<2> ForwardA = bitset<2>("00");
    bitset<2> ForwardB = bitset<2>("00");

    void detectHazard(PipelineRegister pipelineRegister) {

        // MEM 冒险
        if (pipelineRegister.MEM_WB_Register.wrt_enable &&
        (pipelineRegister.MEM_WB_Register.Wrt_reg_addr != bitset<5>("00000")) &&
          (pipelineRegister.MEM_WB_Register.Wrt_reg_addr == 
         								pipelineRegister.ID_EX_Register.Rs1))
            ForwardA = bitset<2>("01");
        
        if (pipelineRegister.MEM_WB_Register.wrt_enable &&
            (pipelineRegister.MEM_WB_Register.Wrt_reg_addr != bitset<5>("00000")) &&
            (pipelineRegister.MEM_WB_Register.Wrt_reg_addr == 
               							pipelineRegister.ID_EX_Register.Rs2))
            ForwardB = bitset<2>("01");
    
    // EX 冒险
        if (pipelineRegister.EX_MEM_Register.wrt_enable &&
        (pipelineRegister.EX_MEM_Register.Rd != bitset<5>("00000")) &&
        (pipelineRegister.EX_MEM_Register.Rd == pipelineRegister.ID_EX_Register.Rs1)) 
            ForwardA = bitset<2>("10");
       
        if (pipelineRegister.EX_MEM_Register.wrt_enable &&
        (pipelineRegister.EX_MEM_Register.Rd != bitset<5>("00000")) &&
        (pipelineRegister.EX_MEM_Register.Rd == pipelineRegister.ID_EX_Register.Rs2)) 
            ForwardB = bitset<2>("10");
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
~~~



- **Load和Store的数据冒险控制单元Hazard Unit**

~~~C++
class HazardUnit{
	public:
    bool hazardDetect(PipelineRegister pipelineRegister){
        return pipelineRegister.ID_EX_Register.Rd == 
            							pipelineRegister.IF_ID_Register.Rs1 ||
               pipelineRegister.ID_EX_Register.Rd == 
            							pipelineRegister.IF_ID_Register.Rs2;
    }
};
~~~



- **逆序执行逻辑**

1. 执行顺序为 WB - MEM - EX - ID - IF
2. 在同一周期每次执行某个部分的时候，需要先将此部分的后续流水线寄存器清空，防止数据多次使用
3. 在清空完成之后，从上个流水线寄存器拿取数据，然后将其放入自身的阶段寄存器，最后再将数据覆写入后续的流水线寄存器

