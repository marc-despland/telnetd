SHELL = /bin/sh
CC    = gcc
  
CFLAGS       = -Wall -Iinclude 
LDLIBS		 = -lutil
TARGET  = telnetd
SOURCES = $(shell echo src/*.c)
HEADERS = $(shell echo include/*.h)
OBJECTS = $(SOURCES:.c=.o)
DEBUGFLAGS = -g
 
all: $(TARGET)
 
$(TARGET): $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) $(LDLIBS) -o $(TARGET) $(OBJECTS)
 
clean:
	-rm -f $(OBJECTS)
	-rm -f gmon.out
 
distclean: clean
	-rm -f $(TARGET)
 
 
.SECONDEXPANSION:
 
$(foreach OBJ,$(OBJECTS),$(eval $(OBJ)_DEPS = $(shell $(CC) $(CFLAGS) -MM $(OBJ:.o=.c) | sed s/.*://)))
%.o: %.c $$($$@_DEPS)
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -c -o $@ $<
  
 
.PHONY : all install uninstall clean distclean
