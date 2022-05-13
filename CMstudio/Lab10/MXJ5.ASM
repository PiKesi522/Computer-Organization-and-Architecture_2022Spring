;流水模型机程序

#LOAD "MXJ5.IS"         ;预调入指令系统/微程序

        org   0
start:  inh   0110h
        ld    r2,0110h
        rrc   r2
        outh  r2
        jmpl  0000h
        end
