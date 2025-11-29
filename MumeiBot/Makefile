
INCLD:=-I./jwt/include -I./uuidv4/include

CPPVER:=-std=c++20

StreamBot: civetweb.o html.o util.o register.o htmlform.o httpheaders.o guild.o channel.o main.o
	g++ -g $^ $(args) $(INCLD) $(CPPVER) -lcurl -lssl -lcrypto -o $@

%.o: %.c
	gcc -g -c $< $(args) $(INCLD) -o $@

%.o: %.cpp
	g++ -g -c $< $(args) $(INCLD) -mavx2 $(CPPVER) -o $@

bin:
	mkdir bin

clean:
	rm civetweb.o html.o util.o register.o htmlform.o httpheaders.o guild.o channel.o main.o
