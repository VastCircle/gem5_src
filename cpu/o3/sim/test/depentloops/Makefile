


make : depentloops.riscv
	@echo "make done"

depentloops.o:depentloops.c
	riscv64-unknown-elf-gcc -O2 -fno-common -fno-builtin-printf -specs=htif.specs -c depentloops.c
depentloops.riscv:depentloops.o
	riscv64-unknown-elf-gcc  -O2 -static -specs=htif_nano.specs depentloops.o -o depentloops.riscv
	rm depentloops.o
	riscv64-unknown-elf-objdump -d depentloops.riscv > depentloops.dump 

gem5:depentloops_gem5.riscv

depentloops_gem5.riscv:depentloops.c
	riscv64-unknown-elf-gcc -O2 -march=rv64gc -mabi=lp64d -static -o depentloops_gem5.riscv depentloops.c 
