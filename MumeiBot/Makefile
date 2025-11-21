


StreamBot: civetweb.o html.o util.o register.o htmlform.o main.o
	g++ -g $^ $(args) -lcurl -o $@

%.o: %.c
	gcc -g -c $< $(args) -o $@

%.o: %.cpp
	g++ -g -c $< $(args) -o $@

bin:
	mkdir bin

clean:
	rm civetweb.o html.o util.o register.o htmlform.o main.o
