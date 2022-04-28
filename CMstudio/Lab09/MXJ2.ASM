;分段模型机程序

#LOAD "MXJ2.IS"         ;预调入指令系统/微程序
#SET RAM 1234h = ABCDh  ;数据区1234H单元预置数据ABCDh

        org   0
start:
        INL   R2
        STA   1234h,R2
        LD    R0,1234h
        OUTH  R0
        JMP   start
        end
