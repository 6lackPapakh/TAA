CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude
TARGET   = protectusb
SRCS     = src/main.cpp src/cipher.cpp src/file_operations.cpp \
           src/usb_detector.cpp src/utils.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
