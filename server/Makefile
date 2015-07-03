TARGET=ws_server

VPATH=\
	.

INCLUDE=\
	-I../libwebsockets/lib \
	-I../libwebsockets

OBJS=\
	 server_app.o

LIBS=\
	 ../libwebsockets/lib/libwebsockets.a

CFLAGS=\
	

LDFLAGS=\
	-lz
	

$(TARGET): $(LIBS) $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) $(LDFLAGS)

.c.o:
	$(CC) -c $< $(INCLUDE) $(CFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

