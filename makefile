CC = mpicc
LDC = mpicc
LD_FLAGS = -lm 
FLAGS =  -lm 
PROG = PA4.cx
OBJS = PA4.o
RM= /bin/rm

OBJS=$(PROG:%.cx=%.o)

all: $(PROG)

$(PROG): $(OBJS)
	$(LDC) $(OBJS) $(LD_FLAGS) -o $(PROG)

%.o: %.c
	$(CC) $(FLAGS) -c $<

clean:
	$(RM) -rf *.o $(PROG)

