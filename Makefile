# Variables
CC = gcc
CFLAGS = -Wall -pthread
TARGETS = ecuafast puerto sri senae supercia

# Regla principal
all: $(TARGETS)

# Reglas individuales para cada ejecutable
ecuafast: ecuafast.c
	$(CC) $(CFLAGS) -o $@ $<

puerto: puerto.c
	$(CC) $(CFLAGS) -o $@ $<

sri: sri.c
	$(CC) $(CFLAGS) -o $@ $<

senae: senae.c
	$(CC) $(CFLAGS) -o $@ $<

supercia: supercia.c
	$(CC) $(CFLAGS) -o $@ $<

# Regla para limpiar archivos generados
clean:
	rm -f $(TARGETS)

# Regla para limpiar todos los archivos temporales
distclean: clean
	rm -f *~
