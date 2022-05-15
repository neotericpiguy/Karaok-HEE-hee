BUILD_PATH=build
EXE=KaraokeApp
PORT=18080

VERSION            = $(shell git describe --tags)
HASH               = $(shell git rev-parse --short HEAD)
DMRCONFIG_VERSION  = $(shell git submodule status)
GITCOUNT           = $(shell git rev-list HEAD --count)

CFLAGS   += -g -O -Wall -Werror -fPIC -MMD -fcommon
CFLAGS   += -DVERSION='"$(VERSION)"'

EFFCFLAGS+=-Wall
EFFCFLAGS+=-Wextra -Werror -Wshadow 
EFFCFLAGS+=-Weffc++ # extreme OCD C++ not recommended
#warn the user if a class with virtual functions has a non-virtual destructor.
#This helps catch hard to track down memory errors
EFFCFLAGS+=-Wnon-virtual-dtor
# EFFCFLAGS+=-Wold-style-cast #bsonheader has oldcasting
EFFCFLAGS+=-Wcast-align
EFFCFLAGS+=-Wunused
EFFCFLAGS+=-Woverloaded-virtual
#warn if you overload (not override) a virtual function
EFFCFLAGS+=-pedantic

CFLAGS   += -std=c++20 $(EFFCFLAGS)
CXXFLAGS += $(CFLAGS)
LIBS     += -lwthttp -lwt

#SRCS=$(shell find src/ -name '*.cpp')
SRCS=$(wildcard src/*.cpp)
OBJS=$(addprefix $(BUILD_PATH)/,$(SRCS:.cpp=.o))
DEPS=$(OBJS:%.o=%.d) 

COMMON_PATH=src/common
COMMON_INCPATHS=$(addprefix -I,$(shell find $(COMMON_PATH) -type d))
COMMON_SRCS=$(shell find $(COMMON_PATH) -iname "*.cpp")
COMMON_OBJS=$(addprefix $(BUILD_PATH)/,$(COMMON_SRCS:.cpp=.o))
DEPS+=$(COMMON_OBJS:%.o=%.d) 
COMMON_LIB=$(BUILD_PATH)/$(COMMON_PATH)/libcommon.a

WIDGETS_PATH=src/widgets
WIDGETS_INCPATHS=$(addprefix -I,$(shell find $(WIDGETS_PATH) -type d))
WIDGETS_SRCS=$(wildcard $(WIDGETS_PATH)/*/*.cpp $(WIDGETS_PATH)/*.cpp)
WIDGETS_OBJS=$(addprefix $(BUILD_PATH)/,$(WIDGETS_SRCS:.cpp=.o))
DEPS+=$(WIDGETS_OBJS:%.o=%.d) 
WIDGETS_LIB=$(BUILD_PATH)/$(WIDGETS_PATH)/libWidgets.a

$(info $$SRCS         = [${SRCS}])
$(info $$OBJS         = [${OBJS}])
$(info $$COMMON_SRCS  = [${COMMON_SRCS}])
$(info $$COMMON_OBJS  = [${COMMON_OBJS}])
$(info $$WIDGETS_SRCS = [${WIDGETS_SRCS}])
$(info $$WIDGETS_OBJS = [${WIDGETS_OBJS}])
$(info $$VERSION      = [${VERSION}])
$(info $$HASH         = [${HASH}])
$(info $$GITCOUNT     = [${GITCOUNT}])
$(info $$BUILD_PATH   = [${BUILD_PATH}])
$(info $$DEPS         = [${DEPS}])
$(info $$CFLAGS       = [${CFLAGS}])
$(info $$CXXFLAGS     = [${CXXFLAGS}])

.phony: run

all: $(BUILD_PATH)/$(EXE)

$(BUILD_PATH)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CC) -o $@ -c $(CFLAGS) $(LIBUSB_INCPATHS) $(COMMON_INCPATHS) $(WIDGETS_INCPATHS) $<

$(COMMON_LIB): $(COMMON_OBJS)
	ar -rcs $@ $^

$(WIDGETS_LIB): $(WIDGETS_OBJS)
	ar -rcs $@ $^

$(BUILD_PATH)/$(EXE): $(OBJS) $(COMMON_LIB) $(WIDGETS_LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

run: $(BUILD_PATH)/$(EXE)
	./$^ 'localhost' --docroot="/usr/share/Wt" --http-listen=0.0.0.0:$(PORT) -c wt_config.xml

server: $(BUILD_PATH)/$(EXE)
	./$^ server

sync: 
	rsync -avx library.csv users.db vizio:./src/Karaok-HEE-hee
	rsync -avx youtube.com_cookies.txt vizio:./src/Karaok-HEE-hee
	rsync -avx /mnt/sdb/karaoke/* vizio:/mnt/sdb/karaoke

clean:
	-rm -rf $(BUILD_PATH)

-include $(DEPS)
