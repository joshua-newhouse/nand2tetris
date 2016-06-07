// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, the
// program clears the screen, i.e. writes "white" in every pixel.

//Initialize position pointer to first memory location of screen
@SCREEN
D=A
@position
M=D

(LOOP)
// Peek at keyboard for key press
@KBD	
D=M

// Jumps to write black routine if a key is being pressed
@WRITE_BLACK
D;JGT

// Write white routine which occurs if the program does not jump to the write black routine on the previous line
@position
A=M
M=0
@SCREEN
D=A
@position
D=M-D
@LOOP
D;JLE
@position
M=M-1
@LOOP
0;JMP

(WRITE_BLACK)
// Writes black to the screen
@position
A=M
M=-1
@24575
D=A
@position
D=D-M
@LOOP
D;JLE
@position
M=M+1
@LOOP
0;JMP
