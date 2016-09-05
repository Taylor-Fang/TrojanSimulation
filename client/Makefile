CXX = G++
TARGET = client
OBJS = client.o
RM = rm -f

$(TARGET):$(OBJS)
	$(CXX) -o $@ $^ -O2
$(OBJS):%.o:%.cpp %.o:%.h
	$(CXX) -c -o $@ $^

clean:
	-$(RM) $OBJS
