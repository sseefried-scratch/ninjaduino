# define the C source files
SRCS = main.c filter.c serial_relay.c cJSON.c line.c  monitor.c trigger.c utils.c worker.c mdwrkapi.c camera.c msgpack_wrapper.c mdcliapi2.c

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

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

%.o : %.c
	$(COMPILE.c)  -MD -o $@ $<
	  @cp $*.d $*.P; \
            sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
            rm -f $*.d

all:  seabeagle

# test: test_filter
test_vector: test_vector.o vector.o

test_trigger: test_trigger.o trigger.o mdcliapi2.o msgpack_wrapper.o utils.o
	gcc -g -o  test_trigger trigger.o test_trigger.o mdcliapi2.o msgpack_wrapper.o utils.o -lzmq -lczmq -lmsgpack
# test_filter: test_filter.o filter.o cJSON.o line.o
#	gcc -o test_filter filter.o test_filter.o cJSON.o line.o -lzmq
#	-lczmq 

seabeagle: $(OBJS)
	gcc -o seabeagle $(OBJS) -lzmq -lczmq -lmsgpack

clean:
	$(RM) *.o *~ $(MAIN)

depend:
	gcc -M $(CFLAGS) $(SRCS)

check-syntax:
	gcc -o nul -S ${CHK_SOURCES}


install: seabeagle
	install seabeagle /usr/bin
	foreman export upstart /etc/init -a ninja
	# this should probably be networking, really
	# but this works and it's late. FIXME
	echo "start on starting ssh" >  n.conf
	echo "stop on runlevel [!2345]" >>  n.conf
	cat /etc/init/ninja.conf >> n.conf
	mv n.conf /etc/init/ninja.conf

