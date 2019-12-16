APP=headje
BIN=$(HOME)/bin

all: $(APP)

$(APP):$(APP).c
	cc -o $(APP) $(APP).c
	strip $(APP)

install:
	cp ./$(APP) $(BIN)

test:
	@sh ./test.sh &
	@sleep 1
	./$(APP) test.txt
	@rm test.txt
