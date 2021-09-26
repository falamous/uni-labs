CFLAGS = -std=c99 -masm=intel -lm -I ./include
OFLAGS = -Ofast -s -frename-registers -funroll-all-loops -finline-functions -fsched-spec
DFLAGS = -O0 -ggdb -DDEBUG
SFLAGS = -fsanitize=address -fsanitize=leak
CC = cc


clean:
	rm *.elf

debug:
	$(eval CFLAGS += ${DFLAGS})
san:
	$(eval CFLAGS += ${SFLAGS})
release:
	$(eval CFLAGS += ${OFLAGS})
linked_stack:
	$(eval CFLAGS += -DLINKED_LIST_STACK)

lab0:
	${CC} ${CFLAGS} -o lab0.elf lab0.c struct/*.c
lab1:
	${CC} ${CFLAGS} -o lab1.elf lab1.c struct/*.c
lab0.5a:
	${CC} ${CFLAGS} -o lab0.5a.elf lab0.5a.c struct/*.c
lab0.5b:
	${CC} ${CFLAGS} -o lab0.6b.elf lab0.5b.c struct/*.c
lab2:
	${CC} ${CFLAGS} -o lab2.elf lab2.c struct/*.c
lab3:
	${CC} ${CFLAGS} -o lab3.elf lab3.c struct/*.c
lab3b:
	${CC} ${CFLAGS} -o lab3b.elf lab3b.c struct/*.c
lab4:
	${CC} ${CFLAGS} -o lab4.elf lab4.c struct/*.c
lab5:
	${CC} ${CFLAGS} -o lab5.elf lab5.c struct/*.c
