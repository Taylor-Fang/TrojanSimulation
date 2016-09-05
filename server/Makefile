CXX = G++
TARGET = server
OBJS = server.o
RM =rm -f

$(TARGET):$(OBJS)
	$(CXX) -o $@ $^ -O2
$(OBJS):%.o:%.cpp %.o:%.h scanipandmac.h synscan.h udpscan.h
	$(CXX) -c -o $@ $^

clean:
	-$(RM) $(OBJS)
