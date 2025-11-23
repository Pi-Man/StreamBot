
INCLD:=-I./jwt/include

StreamBot: civetweb.o html.o util.o register.o htmlform.o main.o
	g++ -g $^ $(args) $(INCLD) -lcurl -lssl -lcrypto -o $@

%.o: %.c
	gcc -g -c $< $(args) $(INCLD) -o $@

%.o: %.cpp
	g++ -g -c $< $(args) $(INCLD) -o $@

bin:
	mkdir bin

clean:
	rm civetweb.o html.o util.o register.o htmlform.o main.o
