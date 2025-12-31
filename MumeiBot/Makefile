
INCLD:=-I./jwt/include -I./uuidv4/include

CPPVER:=-std=c++17

OBJS:=\
civetweb.o \
html.o \
util.o \
register.o \
htmlform.o \
httpheaders.o \
guild.o \
channel.o \
discord.o \
ccscripts.o \
main.o

StreamBot: $(OBJS)
	g++ -g $^ $(args) $(INCLD) $(CPPVER) -lcurl -lpqxx -lpq -lssl -lcrypto -o $@

%.o: %.c
	gcc -g -c $< $(args) $(INCLD) -o $@

%.o: %.cpp
	g++ -g -c $< $(args) $(INCLD) -mavx2 $(CPPVER) -o $@

bin:
	mkdir bin

clean:
	rm $(OBJS)
