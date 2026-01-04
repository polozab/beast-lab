CXXFLAGS=-fast -Mipa=fast -Munroll -Mvect -Minfo -O4 -mp -gopt
CXX?=nvc++

OBJS = main.o

all: multigrid

multigrid: $(OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(OBJS) -o multigrid

clean:
	rm -f *~ *.o *.vtk multigrid

