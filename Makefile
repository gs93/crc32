CXX = g++
CXXFLAGS = -Wall -Wextra -Weffc++ -pedantic
RM = rm -f
CP = cp
SRC = crc32.cpp
LDLIBS = -lboost_regex
DESTDIR = /usr/local

all: crc32

crc32:
	g++ $(LDFLAGS) -o crc32 $(SRC) $(LDLIBS) 

install:
	$(CP) "crc32" "$(DESTDIR)/bin"

uninstall:
	$(RM) "$(DESTDIR)/bin/crc32"

clean:
	$(RM) "crc32"
