CXX = g++
CXXFLAGS = -Wall
LIBS = -lcurl -lcryptopp

VERSION = 0.09
TARGET = bacon

CXXFLAGS += -DBACON_VERSION=\"$(VERSION)\" \
			-DDEFAULT_PROGRAM_NAME=\"$(TARGET)\"

ifeq ($(debug),true)
	CXXFLAGS += -g -DBACON_DEBUG
else
	CXXFLAGS += -O2 -march=native
endif

SOURCES = \
	bacon-actions.cpp \
	bacon-cmd.cpp \
	bacon-device.cpp \
	bacon-devicelist.cpp \
	bacon-env.cpp \
	bacon-file.cpp \
	bacon-htmldoc.cpp \
	bacon-htmlparser.cpp \
	bacon-main.cpp \
	bacon-md5.cpp \
	bacon-net.cpp \
	bacon-progress.cpp \
	bacon-rom.cpp \
	bacon-util.cpp

OBJECTS = \
	bacon-actions.o \
	bacon-cmd.o \
	bacon-device.o \
	bacon-devicelist.o \
	bacon-env.o \
	bacon-file.o \
	bacon-htmldoc.o \
	bacon-htmlparser.o \
	bacon-main.o \
	bacon-md5.o \
	bacon-net.o \
	bacon-progress.o \
	bacon-rom.o \
	bacon-util.o

prefix = /usr/local

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LIBS) -o $(TARGET) $(OBJECTS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

install: $(TARGET)
	install -m 0755 $(TARGET) $(prefix)/bin/$(TARGET)

uninstall:
	rm -f $(prefix)/bin/$(TARGET)

clean:
	@rm -f $(OBJECTS)

clobber:
	@rm -f $(OBJECTS) $(TARGET) *~

.PHONY: install uninstall clean clobber
