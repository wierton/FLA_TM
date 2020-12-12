.PHONY: all

O      ?= build
CFILES := main.cc
OFILES := $(CFILES:%.cc=$(O)/%.o)
APP    := turing

$(O)/%.o: %.cc
	mkdir -p $(@D)
	g++ -g -MMD -c $^ -o $@

all: $(APP)
$(APP): $(OFILES)
	g++ $^ -o $@

run: $(APP)
	./$< test/* 1001001

test-case%:
	mkdir -p $(O)
	g++ test/$@.cc -o $(O)/$@
	./$(O)/$@

-include $(OFILES:.o=.d)

clean:
	rm $(APP)
	rm -rf $(O)
