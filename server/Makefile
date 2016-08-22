CC = G++
TARGET = server
OBJS = server.o
RM =rm -f

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ -O2
$(OBJS):%.o:%.cpp %.o:%.h scanipandmac.h synscan.h udpscan.h
	$(CC) -c -o $@ $^

clean:
	-$(RM) $(OBJS)