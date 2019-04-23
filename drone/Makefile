# This is a general use makefile for robotics cape projects written in C.
# Just change the target name to match your main source code filename.
TARGET = QC

LIBDIR_APP_LOADER?=../../app_loader/lib
INCDIR_APP_LOADER?=../../app_loader/include


CC		    := g++
LINKER		:= g++ -o
CFLAGS		:= -c -Wall -g -std=c++0x -I$./
LFLAGS		:= -Wl,--no-as-needed -lm -lrt -lpthread -lroboticscape -lprussdrv

SOURCES		:= $(wildcard *.cpp)
INCLUDES	:= $(wildcard *.h)
OBJECTS		:= $(SOURCES:$%.cpp=$%.o)
PRUSS       := range.bin

prefix		:= /usr/local
RM          := rm -f
MV          := mv
INSTALL		:= install -m 4755
INSTALLDIR	:= install -d -m 755 

LINK		:= ln -s -f
LINKDIR		:= /etc/roboticscape
LINKNAME	:= link_to_startup_program


# linking Objects

$(TARGET): $(OBJECTS)
	pasm -b range.p
	$(LINKER) $(@) $(OBJECTS) $(LFLAGS)
	@$(MV) *.o ./obj


# compiling command
$(OBJECTS): %.o : %.cpp $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $(@)
	@echo "Compiled: "$<

all: 
	$(TARGET)

debug:
	$(MAKE) $(MAKEFILE) DEBUGFLAG="-g -D DEBUG"
	@echo " "
	@echo "$(TARGET) Make Debug Complete"
	@echo " "

install:
	$(MAKE) --no-print-directory
	$(INSTALLDIR) $(DESTDIR)$(prefix)/bin
	$(INSTALL) $(TARGET) $(DESTDIR)$(prefix)/bin
	@echo "$(TARGET) Install Complete"

clean:
	$(RM) $(OBJECTS)
	$(RM) $(TARGET)
	@echo "$(TARGET) Clean Complete"

uninstall:
	$(RM) $(DESTDIR)$(prefix)/bin/$(TARGET)
	@echo "$(TARGET) Uninstall Complete"

runonboot:
	$(MAKE) install --no-print-directory
	$(LINK) $(DESTDIR)$(prefix)/bin/$(TARGET) $(LINKDIR)/$(LINKNAME)
	@echo "$(TARGET) Set to Run on Boot"

