CXX = g++
CXXFLAGS = -std=c++17 -Isrc -Wall -Wextra
SRC = src/core/core.cpp src/models/ModelBackend.cpp src/utils/json_utils.cpp src/events/events.cpp
TEST_SRC = test/unit/test_model_backend.cpp

all: quantalista

quantalista: $(SRC) src/main.cpp
	$(CXX) $(CXXFLAGS) $(SRC) src/main.cpp -o quantalista

test: $(TEST_SRC) $(SRC)
	$(CXX) $(CXXFLAGS) $(TEST_SRC) $(SRC) -o run_tests
	./run_tests

clean:
	rm -f quantalista run_tests
