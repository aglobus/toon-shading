LDFLAGS = -lGLU -lglut -lGLEW -lGL
CXXFLAGS = -O2 -Wno-write-strings -DLINUX

PROG = shader

OBJS = shader.o gpuProgram.o linalg.o wavefront.o renderer.o gbuffer.o font.o

$(PROG): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS) 

clean:
	rm -f *.o *~ $(PROG)

depend:	
	makedepend -Y *.h *.cpp

gpuProgram.o: headers.h linalg.h
renderer.o: wavefront.h headers.h seq.h linalg.h shadeMode.h gpuProgram.h
renderer.o: gbuffer.h
seq.o: headers.h
wavefront.o: headers.h seq.h linalg.h shadeMode.h gpuProgram.h
font.o: headers.h
gbuffer.o: headers.h gbuffer.h
gpuProgram.o: gpuProgram.h headers.h linalg.h
linalg.o: linalg.h
renderer.o: headers.h renderer.h wavefront.h seq.h linalg.h shadeMode.h
renderer.o: gpuProgram.h gbuffer.h shader.h
shader.o: headers.h linalg.h wavefront.h seq.h shadeMode.h gpuProgram.h
shader.o: renderer.h gbuffer.h font.h
wavefront.o: headers.h gpuProgram.h linalg.h wavefront.h seq.h shadeMode.h
