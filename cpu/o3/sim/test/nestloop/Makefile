


make : nestloop.riscv
	@echo "make done"

nestloop.o:nestloop.c
	riscv64-unknown-elf-gcc -O2 -fno-common -fno-builtin-printf -specs=htif.specs -c nestloop.c
nestloop.riscv:nestloop.o
	riscv64-unknown-elf-gcc  -O2 -static -specs=htif_nano.specs nestloop.o -o nestloop.riscv
	rm nestloop.o
	riscv64-unknown-elf-objdump -d nestloop.riscv > nestloop.dump 
