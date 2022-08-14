# file: isr_timer.s

.section isr
# prekidna rutina za tajmer
.global isr_timer
isr_timer:
  iret

.end
