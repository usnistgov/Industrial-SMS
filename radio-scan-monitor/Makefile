###
Name := sdr_processor
CXX := g++
CXXFLAGS := -O2 -Wall
Libs := -lSDL2 -lSDL2_ttf 

$(Name): main.cpp
	$(CXX) -o $(Name) main.cpp $(Libs) $(CXXFLAGS)

clean: 
	rm $(Name)
