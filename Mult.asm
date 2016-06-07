// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

@R2
M=0
@R0
D=M
@R1
D=D-M

@LOOP
 D;JLT
@SWAP_R0_R1
 0;JMP

(SWAP_R0_R1)
 @R0
 D=M
 @R3
 M=D

 @R1
 D=M
 @R0
 M=D

 @R3
 D=M
 @R1
 M=D
 @LOOP
  0;JMP

(LOOP)
 @R3
 D=M
 @END
  D;JLE

 @R2
 D=M
 @R1
 D=D+M
 @R2
 M=D
 @R3
 M=M-1
 @LOOP
  0;JMP

(END)
 @END
  0;JMP
