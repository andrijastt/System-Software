#include <regex.h>
#include <iostream>


/*

Asemblerske direktive
.global <lista_simbola>
.extern <lista_simbola>
.section <ime_sekcije>
.word <lista_simbola_ili_literal>
.skip <literal>
.end

Asemblerske naredbe
halt Zaustavlja izvršavanje instrukcija -
int regD push psw; pc <= mem16[(regD mod 8)*2]; -
iret pop psw; pop pc; psw
call operand push pc; pc <= operand; -
ret pop pc; -
jmp operand pc <= operand; -
jeq operand if (equal) pc <= operand; -
jne operand if (not equal) pc <= operand; -
jgt operand if (signed greater) pc <= operand; -
push regD sp <= sp - 2; mem16[sp] <= regD; -
pop regD regD <= mem16[sp]; sp <= sp + 2; -
xchg regD, regS temp <= regD; regD <= regS; regS <= temp; -
add regD, regS regD <= regD + regS; -
sub regD, regS regD <= regD - regS; -
mul regD, regS regD <= regD * regS; -
div regD, regS regD <= regD / regS; -
cmp regD, regS temp <= regD - regS; Z O C N
not regD regD <= ~regD; -
and regD, regS regD <= regD & regS; -
or regD, regS regD <= regD | regS -
xor regD, regS regD <= regD ^ regS; -
test regD, regS temp <= regD & regS; Z N
shl regD, regS regD <= regD << regS; Z C N
shr regD, regS regD <= regD >> regS; Z C N
ldr regD, operand regD <= operand; -
str regD, operand operand <= regD; -

registri r0 , r1 , r2 , r3 , r4 , r5 , r6 / sp , r7 / pc i psw

Asemblerske naredbe za rad sa podacima podržavaju različite sintaksne notacije za operand:
• $<literal> - vrednost <literal>
• $<simbol> - vrednost <simbol>
• <literal> - vrednost iz memorije na adresi <literal>
• <simbol> - vrednost iz memorije na adresi <simbol> apsolutnim adresiranjem
• %<simbol> - vrednost iz memorije na adresi <simbol> PC relativnim adresiranjem
• <reg> - vrednost u registru <reg>
• [<reg>] - vrednost iz memorije na adresi <reg>
• [<reg> + <literal>] - vrednost iz memorije na adresi <reg> + <literal>
• [<reg> + <simbol>] - vrednost iz memorije na adresi <reg> + <simbol>

Asemblerske naredbe skoka i poziva potprograma podržavaju različite sintaksne notacije za operand:
• <literal> - vrednost <literal>
• <simbol> - vrednost <simbol> apsolutnim adresiranjem
• %<simbol> - vrednost <simbol> PC relativnim adresiranjem
• *<literal> - vrednost iz memorije na adresi <literal>
• *<simbol> - vrednost iz memorije na adresi <simbol>
• *<reg> - vrednost u registru <reg>
• *[<reg>] - vrednost iz memorije na adresi <reg>
• *[<reg> + <literal>] - vrednost iz memorije na adresi <reg> + <literal>
• *[<reg> + <simbol>] - vrednost iz memorije na adresi <reg> + <simbol>

*/

// regex()

class Assembler{

public:

private:

};