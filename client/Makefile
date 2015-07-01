TARGET=ws_client

VPATH=\
	.

INCLUDE=\
	../libwebsockets/lib

OBJS=\
	 client_app.o

LIBS=\
	 ../libwebsockets/lib/libwebsockets.a

CFLAGS=\
	-g

LDFLAGS=\
	-lz
	

$(TARGET): $(LIBS) $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) $(LDFLAGS)

.c.o:
	$(CC) -c $< $(INCLUDE) $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

