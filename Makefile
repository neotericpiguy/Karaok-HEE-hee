BUILD_PATH=build

VERSION            = $(shell git describe --tags --abbrev=0)
HASH               = $(shell git rev-parse --short HEAD)
DMRCONFIG_VERSION  = $(shell git submodule status)
GITCOUNT           = $(shell git rev-list HEAD --count)

CXXFLAGS=-std=c++20 -Weffc++

SRCS=$(shell find src/ -name '*.cpp')
OBJS=$(addprefix $(BUILD_PATH)/,$(SRCS:.cpp=.o))

$(info $$SRCS       = [${SRCS}])
$(info $$OBJS       = [${OBJS}])
$(info $$VERSION    = [${VERSION}])
$(info $$BUILD_PATH = [${BUILD_PATH}])

.phony: run

$(BUILD_PATH)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CC) -o $@ -c $(CFLAGS) $(LIBUSB_INCPATHS) $<

./build/hello: $(OBJS)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^ -lwthttp -lwt

run: build/hello
	./build/hello --docroot=./ --http-listen=0.0.0.0:18080

clean:
	-rm -rf build
