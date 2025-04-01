


make : unrolledloop.riscv
	@echo "make done"

unrolledloop.o:unrolledloop.c
	riscv64-unknown-elf-gcc -O2 -fno-common -fno-builtin-printf -specs=htif.specs -c unrolledloop.c
unrolledloop.riscv:unrolledloop.o
	riscv64-unknown-elf-gcc  -O2 -static -specs=htif_nano.specs unrolledloop.o -o unrolledloop.riscv
	rm unrolledloop.o
	riscv64-unknown-elf-objdump -d unrolledloop.riscv > unrolledloop.dump 
