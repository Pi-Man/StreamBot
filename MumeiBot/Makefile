


StreamBot: civetweb.o html.o util.o register.o main.o
	gcc -g $^ -D_DEBUG -lcurl -o $@

%.o: %.c
	gcc -g -c $< -D_DEBUG -o $@

bin:
	mkdir bin

clean:
	rm civetweb.o html.o util.o register.o main.o
