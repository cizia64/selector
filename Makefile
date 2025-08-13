NAME=selector

SRCS := main.cpp Selector.cpp
OBJS := $(SRCS:.cpp=.o)

CXX := g++
CXXFLAGS :=  -ggdb3 -O0 --std=c++17 -Werror -Wextra -Wall -Wpedantic
TARGET_ARCH := -march=armv8-a+crc+crypto+simd -mtune=cortex-a53 -mcpu=cortex-a53+crc 
LDFLAGS := `sdl2-config --libs --cflags`
LDLIBS := -lm -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(NAME)

trimui: $(OBJS)
	$(CXX) $(CXXFLAGS) $(TARGET_ARCH) $(OBJS) $(LDFLAGS) $(LDLIBS) -DTRIMUI -o $(NAME)

all: $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re trimui
