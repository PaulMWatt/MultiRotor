CROSS_COMPILE?=arm-linux-gnueabihf-

LIBDIR_APP_LOADER?=../../app_loader/lib
INCDIR_APP_LOADER?=../../app_loader/include
#BINDIR?=../bin
BINDIR=./

#CFLAGS+= -Wall -I$(INCDIR_APP_LOADER) -D__DEBUG -O2 -mtune=cortex-a8 -march=armv7-a
#LDFLAGS+=-L$(LIBDIR_APP_LOADER) 

CFLAGS+= -Wall -I$./ -D__DEBUG -O2 -mtune=cortex-a8 -march=armv7-a
OBJDIR=obj
TARGET=$(BINDIR)/IMU

#_DEPS =
#DEPS = $(patsubst %,$(INCDIR_APP_LOADER)/%,$(_DEPS))
DEPS = IMU_test.cpp IMU.cpp I2CBus.cpp
#DEPS = IMU_test.cpp

_OBJ = IMU_test.o IMU.o I2CBus.o
#_OBJ = IMU_test.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))



$(OBJDIR)/%.o: %.cpp $(DEPS)
	@mkdir -p obj
	$(CROSS_COMPILE)g++ $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJ)
	$(CROSS_COMPILE)g++ $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf $(OBJDIR)/ *~  $(INCDIR_APP_LOADER)/*~  $(TARGET)
