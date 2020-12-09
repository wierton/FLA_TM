.PHONY: all

O      ?= build
CFILES := main.cc
OFILES := $(CFILES:%.cc=$(O)/%.o)
APP    := turing

$(O)/%.o: %.cc
	mkdir -p $(@D)
	g++ -MMD -c $^ -o $@

all: $(APP)
$(APP): $(OFILES)
	g++ $^ -o $@

-include $(OFILES:.o=.d)

clean:
	rm $(APP)
	rm -rf $(O)
