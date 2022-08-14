# file: isr_terminal.s

.section isr
# prekidna rutina za terminal
.global isr_terminal
isr_terminal:
  iret
  
.end
