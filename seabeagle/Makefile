# define the C source files
SRCS = line_dispatcher.c serial_relay.c cJSON.c line.c  monitor.c trigger.c utils.c worker.c mdwrkapi.c camera.c msgpack_wrapper.c mdcliapi2.c identity.c config.c

TESTSRCS=test_trigger.c test_config.c test_line_dispatcher.c test_json.c test_line.c
TESTOBJS = $(TESTSRCS:.c=.o)

OBJS = $(SRCS:.c=.o)
CFLAGS=-g -Wall -std=c99
LDFLAGS=-lzmq -lczmq -lmsgpack -lsqlite3
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

# i have absolutely no idea what this does...
# %.o : %.c
# 	$(COMPILE.c)  -MD -o $@ $<
# 	  @cp $*.d $*.P; \
#             sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
#                 -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
#             rm -f $*.d

all:  seabeagle

seabeagle: $(OBJS) main.o
	gcc -o seabeagle main.o $(OBJS) $(LDFLAGS)

clean:
	$(RM) *.o *~ $(MAIN)

depend:
	gcc -M $(CFLAGS) $(SRCS)

check-syntax:
	gcc -o nul -S ${CHK_SOURCES}

tester: tester.o $(TESTOBJS) $(OBJS)
	gcc -g -o  tester tester.o $(TESTOBJS) $(OBJS) $(LDFLAGS)

test: tester
	./tester

# this should probably be networking, really
# but this works and it's late. FIXME
install: seabeagle
	install seabeagle /usr/bin
	foreman export upstart /etc/init -a ninja -u ninja -d /tmp
	echo "start on starting ssh" >  n.conf
	echo "stop on runlevel [!2345]" >>  n.conf
	cat /etc/init/ninja.conf >> n.conf
	mv n.conf /etc/init/ninja.conf

