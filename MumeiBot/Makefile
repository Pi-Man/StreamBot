
INCLD:=-I./jwt/include -I./uuidv4/include

StreamBot: civetweb.o html.o util.o register.o htmlform.o httpheaders.o main.o
	g++ -g $^ $(args) $(INCLD) -lcurl -lssl -lcrypto -o $@

%.o: %.c
	gcc -g -c $< $(args) $(INCLD) -o $@

%.o: %.cpp
	g++ -g -c $< $(args) $(INCLD) -mavx2 -o $@

bin:
	mkdir bin

clean:
	rm civetweb.o html.o util.o register.o htmlform.o httpheaders.o main.o
