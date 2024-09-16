# Makefile

CC = mpicxx
CFLAGS =
LDFLAGS = -lm
EXECUTABLE = Output
NP = 4

.PHONY: all run clean

all: $(EXECUTABLE) run

$(EXECUTABLE): main.cpp
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

run: $(EXECUTABLE)
	@echo "Number of processes: $(NP)"
	@echo "Logical Clock Start Time : Random"
	@echo "Sleep: 2s"
	mpirun -oversubscribe -np $(NP) ./$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)
