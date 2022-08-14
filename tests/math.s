# file: math.s

.global mathAdd, mathSub, mathMul, mathDiv

.section math
mathAdd:
  push r1
  ldr r0, [r6 + 4]
  ldr r1, [r6 + 6]
  add r0, r1
  pop r1
  ret

mathSub:
  push r1
  ldr r0, [r6 + 4]
  ldr r1, [r6 + 6]
  sub r0, r1
  pop r1
  ret

mathMul:
  push r1
  ldr r0, [r6 + 4]
  ldr r1, [r6 + 6]
  mul r0, r1
  pop r1
  ret

mathDiv:
  push r1
  ldr r0, [r6 + 4]
  ldr r1, [r6 + 6]
  div r0, r1
  pop r1
  ret

.end
