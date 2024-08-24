#For C++ change to -std=c++20
CFLAGS=-Wall -Wextra -pedantic -g -std=c99 -Og -fsanitize=undefined \
	   -D_DEFAULT_SOURCE
FFLAGS=
LFLAGS=-lubsan
TARGET=tests


%.o : %.c
	$(CC) $(CFLAGS) $(FFLAGS) -c $<

$(TARGET) : main.o petri.o
	$(CC) -o $@ $^ $(LFLAGS)

runtests: $(TARGET)
runtests:
	./$(TARGET)

clean:
	$(RM) $(TARGET) *.o

release: CFLAGS=-Wall -Wextra -pedantic -g -std=c99 -O2 -DNDEBUG \
	   -D_DEFAULT_SOURCE
release: LFLAGS=
release: clean
release: $(TARGET)

lint:
	cppcheck --enable=warning,style,performance,portability,unusedFunction .

