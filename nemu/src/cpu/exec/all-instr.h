#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

//PA2.1 added
make_EHelper(pop);
make_EHelper(push);
make_EHelper(call);
make_EHelper(ret);
make_EHelper(xor);
make_EHelper(sub);
make_EHelper(lea);
make_EHelper(and);
make_EHelper(nop);
