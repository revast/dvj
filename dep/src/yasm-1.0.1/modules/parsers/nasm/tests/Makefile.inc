# $Id: Makefile.inc 2325 2010-05-16 06:30:24Z peter $

TESTS += modules/parsers/nasm/tests/nasm_test.sh

EXTRA_DIST += modules/parsers/nasm/tests/nasm_test.sh
EXTRA_DIST += modules/parsers/nasm/tests/alignnop16.asm
EXTRA_DIST += modules/parsers/nasm/tests/alignnop16.hex
EXTRA_DIST += modules/parsers/nasm/tests/alignnop32.asm
EXTRA_DIST += modules/parsers/nasm/tests/alignnop32.hex
EXTRA_DIST += modules/parsers/nasm/tests/charconstmath.asm
EXTRA_DIST += modules/parsers/nasm/tests/charconstmath.hex
EXTRA_DIST += modules/parsers/nasm/tests/dirwarning.asm
EXTRA_DIST += modules/parsers/nasm/tests/dirwarning.errwarn
EXTRA_DIST += modules/parsers/nasm/tests/dirwarning.hex
EXTRA_DIST += modules/parsers/nasm/tests/dy.asm
EXTRA_DIST += modules/parsers/nasm/tests/dy.hex
EXTRA_DIST += modules/parsers/nasm/tests/endcomma.asm
EXTRA_DIST += modules/parsers/nasm/tests/endcomma.hex
EXTRA_DIST += modules/parsers/nasm/tests/equcolon.asm
EXTRA_DIST += modules/parsers/nasm/tests/equcolon.hex
EXTRA_DIST += modules/parsers/nasm/tests/equlocal.asm
EXTRA_DIST += modules/parsers/nasm/tests/equlocal.hex
EXTRA_DIST += modules/parsers/nasm/tests/hexconst.asm
EXTRA_DIST += modules/parsers/nasm/tests/hexconst.hex
EXTRA_DIST += modules/parsers/nasm/tests/long.asm
EXTRA_DIST += modules/parsers/nasm/tests/long.hex
EXTRA_DIST += modules/parsers/nasm/tests/locallabel.asm
EXTRA_DIST += modules/parsers/nasm/tests/locallabel.hex
EXTRA_DIST += modules/parsers/nasm/tests/locallabel2.asm
EXTRA_DIST += modules/parsers/nasm/tests/locallabel2.hex
EXTRA_DIST += modules/parsers/nasm/tests/nasm-prefix.asm
EXTRA_DIST += modules/parsers/nasm/tests/nasm-prefix.hex
EXTRA_DIST += modules/parsers/nasm/tests/newsect.asm
EXTRA_DIST += modules/parsers/nasm/tests/newsect.hex
EXTRA_DIST += modules/parsers/nasm/tests/orphannowarn.asm
EXTRA_DIST += modules/parsers/nasm/tests/orphannowarn.hex
EXTRA_DIST += modules/parsers/nasm/tests/prevlocalwarn.asm
EXTRA_DIST += modules/parsers/nasm/tests/prevlocalwarn.errwarn
EXTRA_DIST += modules/parsers/nasm/tests/prevlocalwarn.hex
EXTRA_DIST += modules/parsers/nasm/tests/strucalign.asm
EXTRA_DIST += modules/parsers/nasm/tests/strucalign.hex
EXTRA_DIST += modules/parsers/nasm/tests/struczero.asm
EXTRA_DIST += modules/parsers/nasm/tests/struczero.hex
EXTRA_DIST += modules/parsers/nasm/tests/syntax-err.asm
EXTRA_DIST += modules/parsers/nasm/tests/syntax-err.errwarn
EXTRA_DIST += modules/parsers/nasm/tests/uscore.asm
EXTRA_DIST += modules/parsers/nasm/tests/uscore.hex

EXTRA_DIST += modules/parsers/nasm/tests/worphan/Makefile.inc

include modules/parsers/nasm/tests/worphan/Makefile.inc
