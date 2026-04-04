CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude
LDFLAGS  = -lssl -lcrypto
TARGET   = protectusb_menu
SRCS     = src/menu_main.cpp src/menu_cipher.cpp src/file_operations.cpp \
           src/usb_detector.cpp src/utils.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
