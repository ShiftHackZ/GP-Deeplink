CC = g++
CFLAGS = -std=c++17
LIBS = 

MODELS_DIR = models
BUILD_DIR = build
TEST_GPDEEPLINK_DIR = docs
TARGET = gpdeeplink

SRCS := $(wildcard ./*.cpp)
OBJS := $(patsubst ./%.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

-include $(DEPS)

$(BUILD_DIR)/%.o: ./%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_GPDEEPLINK_DIR)

.PHONY: clean
