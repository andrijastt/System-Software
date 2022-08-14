# file: ivt.s

.extern isr_reset, isr_timer, isr_terminal, isr_user0

.section ivt
.word isr_reset
.skip 2 # isr_error
.word isr_timer
.word isr_terminal
.word isr_user0
.skip 6
.end
