# define the C source files
SRCS = filter.c serial_relay.c cJSON.c line.c  monitor.c trigger.c utils.c worker.c mdwrkapi.c camera.c msgpack_wrapper.c mdcliapi2.c identity.c config.c

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)
#CFLAGS=-std=c99
CFLAGS=-g -Wall
LDFLAGS=-lzmq -lczmq -lmsgpack -lsqlite3
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

%.o : %.c
	$(COMPILE.c)  -MD -o $@ $<
	  @cp $*.d $*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
            rm -f $*.d

all:  seabeagle

# test_filter: test_filter.o filter.o cJSON.o line.o
#	gcc -o test_filter filter.o test_filter.o cJSON.o line.o -lzmq
#	-lczmq 

seabeagle: $(OBJS) main.o
	gcc -o seabeagle main.o $(OBJS) $(LDFLAGS)

clean:
	$(RM) *.o *~ $(MAIN)

depend:
	gcc -M $(CFLAGS) $(SRCS)

check-syntax:
	gcc -o nul -S ${CHK_SOURCES}

tester: tester.o test_trigger.o $(OBJS)
	gcc -g -o  tester tester.o test_trigger.o $(OBJS) $(LDFLAGS)

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

