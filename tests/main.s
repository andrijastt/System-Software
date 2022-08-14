# file: main.s

.extern mathAdd, mathSub, mathMul, mathDiv

.global my_start

.global value0, value1, value2, value3, value4, value5, value6

.section my_code
my_start:
  ldr r6, $0xFEFE # init SP

  ldr r0, $4 # ivt entry number for isr_user0 
  int r0

  ldr r0, $0
  push r0
  ldr r0, $1
  push r0
  call mathAdd # pc <= mathAdd
  str r0, value1

  ldr r0, $1
  push r0
  ldr r0, $1
  push r0
  call %mathAdd # pc <= pc + (mathAdd - pc) ~ mathAdd
  str r0, value2

  ldr r0, $8
  push r0
  ldr r0, $11
  push r0
  ldr r0, $2
  ldr r1, $destinations
  add r0, r1
  call *[r0] # pc <= mem16[r0] ~ mem16[2 + destinations] ~ mathSub
  str r0, value3

  ldr r0, $2
  push r0
  ldr r0, $2
  push r0
  ldr r0, $4
  call *[r0 + destinations] # pc <= mem16[r0 + destinations] ~ mathMul
  str r0, value4

  ldr r0, $5
  push r0
  ldr r0, $25
  push r0
  ldr r0, $6
  ldr r1, $destinations
  add r0, r1
  ldr r0, [r0]
  call *r0 # pc <= r0 ~ mem16[6 + destinations] ~ mathDiv
  str r0, value5

  ldr r0, value0
  ldr r1, value1
  ldr r2, value2
  ldr r3, value3
  ldr r4, value4
  ldr r5, value5
  ldr r6, value6

  halt

.section my_data
value0:
.word 0
value1:
.word 0
value2:
.word 0
value3:
.word 0
value4:
.word 0
value5:
.word 0
value6:
.word 0
destinations:
.word mathAdd
.word mathSub
.word mathMul
.word mathDiv

.end
