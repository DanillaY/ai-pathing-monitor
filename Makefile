CXX = g++
CXXFLAGS = -fPIC -O2 -shared -std=c++20

INCLUDES = -Iimgui -Iimgui/backends -I/usr/include/SDL2

SRC = main.cpp \
    overlay.cpp \
    shared.cpp \
    mem.cpp \
    read.S \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_widgets.cpp \
    imgui/imgui_tables.cpp \
    imgui/backends/imgui_impl_sdl2.cpp\
    imgui/backends/imgui_impl_opengl3.cpp

LIBS = -lSDL2 -lGL -lGLEW -ldl

TARGET = overlay.so

all: imgui $(TARGET)

imgui:
	@if [ ! -d "imgui" ]; then \
		echo "Clonning imgui submodule"; \
		git submodule update --init --recursive; \
	fi

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(INCLUDES) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)