CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -g
LIBS = `pkg-config --libs gtk+-3.0`

all:
	$(CC) main.c sns.c -o sns_gtk $(CFLAGS) $(LIBS)

clean:
	rm -f sns_gtk.exe sns_gtk
