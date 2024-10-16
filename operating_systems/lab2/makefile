TARGETS = child gates

# Targets to build when running `make`
all: $(TARGETS)

# Phony targets. Executed always.
.PHONY: all clean

# Targets to build `child` binary
child: child.o
	gcc child.o -o child
child.o: child.c
	gcc child.c -c -o child.o

# Targets to build `gates` binary
gates: gates.o
	gcc gates.o -o gates
gates.o: gates.c
	gcc gates.c -c -o gates.o

# Run `make clean` to delete binaries and object files.
clean:
	rm -rf $(TARGETS) *.o
