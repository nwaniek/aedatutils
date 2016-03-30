CC	  	= clang
CXX	  	= clang++
LD	  	= ld
CXX_STD	  	= c++14
WARNINGS  	= -Wall -Werror -pedantic
CFLAGS	  	= -O3 -ffast-math
LDFLAGS	  	= ${LIBS}
OBJ_FILES   	= $(SRC:.cpp=.o)

SRC		= main.cpp

INCLUDES  	=

LIBS	  	=

all: aedatinfo

aedatinfo: $(OBJ_FILES)
	${CXX} ${LDFLAGS} $^ -o $@

%.o: %.cpp
	${CXX} ${CFLAGS} ${WARNINGS} ${INCLUDES} -std=${CXX_STD} -c $<

clean:
	rm -rf main.o
