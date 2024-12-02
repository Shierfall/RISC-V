.text
.globl _start
_start:
    li a0, 0              # Load status code into a0
    li a7, 93             # Load ecall number for exit with status into a7
    ecall                 # Exit the program
