NAME = selector

# Toolchain
CXX := /opt/aarch64-linux-gnu-7.5.0-linaro/bin/aarch64-linux-gnu-g++
SYSROOT := /opt/aarch64-linux-gnu-7.5.0-linaro/sysroot

# CPU & optimisation
CXXFLAGS := -std=c++17 -O3 -march=armv8-a -mtune=cortex-a53 \
            -DRESDIR=\"res\" -D_REENTRANT -Werror -Wextra -Wall -Wpedantic -DTRIMUI
TARGET_ARCH := --sysroot=$(SYSROOT)

# Includes
INCLUDES := -I$(SYSROOT)/usr/include/SDL2 -I./src/extern/rotozoom

# Sources
CPP_SRCS := $(wildcard *.cpp)
C_SRCS   := $(wildcard src/extern/rotozoom/*.c)
OBJS     := $(CPP_SRCS:.cpp=.o) $(C_SRCS:.c=.o)

# Libraries (dynamic linking)
LIBS := -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lpthread -lm -lz -ldl

# Compile C++ files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(TARGET_ARCH) $(INCLUDES) -c $< -o $@

# Compile C files
%.o: %.c
	$(CXX) $(CXXFLAGS) $(TARGET_ARCH) $(INCLUDES) -c $< -o $@

# Link target
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(TARGET_ARCH) $(INCLUDES) $(OBJS) $(LIBS) -o $(NAME)

# Shortcuts
all: $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
