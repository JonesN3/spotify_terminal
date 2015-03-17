CC      := gcc
PKGS	:= libspotify alsa
CFLAGS  := -g -Wall  `pkg-config --cflags $(PKGS)`
LIBS    := `pkg-config --libs $(PKGS)` -lpthread 

TARGET	:= spot
SOURCES := $(wildcard  src/*.c )
OBJECTS := $(patsubst src/%,build/%,$(SOURCES:.c=.o))
DEPS	:= $(OBJECTS:.o=.deps)

$(TARGET): $(OBJECTS)
	@echo "  Linking..."; $(CC) $^ -o $(TARGET) $(LIBS)

build/%.o: src/%.c
	@mkdir -p build/
	@echo "  CC $<"; $(CC) $(CFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	@echo "  Cleaning..."; $(RM) -r build/ $(TARGET)

debug: CFLAGS+=-DDEBUG -g

-include $(DEPS)

.PHONY: clean
