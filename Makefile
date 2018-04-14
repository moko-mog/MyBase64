SRC=Base64.c
PROG=$(SRC:.c=.exe)
OBJ=$(SRC:.c=.obj)
OBJS=$(OBJ)
BINS=$(PROG) $(OBJ)
CFLAGS=/nologo /c
#User32.lib => CreateWindow , ShowWindow , RegisterClass
#Gdi32.lib => GetStockObject
LDFLAGS=/NOLOGO
LIBS=
#LIBS=User32.lib Gdi32.lib

CC=cl.exe
LD=link.exe

#.SUFFIXES: .c .obj
.c.obj:
	$(CC) $(CFLAGS) /Fo$(@F) $**


$(PROG): $(OBJ)
	$(LD) $(LDFLAGS) /OUT:$(@F) $(LIBS) $**
	
$(OBJ): $(SRC)

	
all: $(PROG)

clean:
	del $(BINS)
	
clear:
	del $(OBJS)