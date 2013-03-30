TARGET=bdiff
C_FLAG=-Wall
$(TARGET): $(TARGET).o
	gcc -o $(TARGET) $(TARGET).c $(C_FLAG)

clean:
	rm $(TARGET) *.o
