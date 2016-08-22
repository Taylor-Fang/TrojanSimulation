CC = G++
TARGET = client
OBJS = client.o
RM = rm -f

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ -O2
$(OBJS):%.o:%.cpp %.o:%.h
	$(CC) -c -o $@ $^

clean:
	-$(RM) $OBJS