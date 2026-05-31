CXX = g++
CXXFLAGS = -std=c++17 -Isrc -Wall -Wextra
SRC = src/core/core.cpp src/models/ModelBackend.cpp src/utils/json_utils.cpp src/events/events.cpp
TEST_SRC = test/unit/test_model_backend.cpp

BRIDGE_TEST_SRC = test/integration/bridge_tests.cpp src/core/core.cpp src/models/ModelBackend.cpp src/utils/json_utils.cpp src/events/events.cpp

bridge_test: $(BRIDGE_TEST_SRC)
	$(CXX) $(CXXFLAGS) $(BRIDGE_TEST_SRC) -o run_bridge_tests
	./run_bridge_tests

# Includes components from all repositories for true E2E testing
# Extract paths from .quanta
QUANTA_ETHOS_DIR := $(shell grep 'quanta_ethos.path=' .quanta | cut -d'=' -f2)
QUANTA_TISSU_DIR := $(shell grep 'quanta_tissu.path=' .quanta | cut -d'=' -f2)
QUANTA_HABA_DIR := $(shell grep 'quanta_haba.path=' .quanta | cut -d'=' -f2)
QUANTA_GLIA_DIR := $(shell grep 'quanta_glia.path=' .quanta | cut -d'=' -f2)

REAL_INT_SRC = test/integration/real_integration_tests.cpp \
               src/core/core.cpp \
               src/models/ModelBackend.cpp \
               src/utils/json_utils.cpp \
               src/events/events.cpp \
               $(QUANTA_ETHOS_DIR)/src/QuantaEthos/ethics_logic.cpp \
               $(QUANTA_TISSU_DIR)/tisslm/compiler/parser.cpp \
               $(QUANTA_HABA_DIR)/src/c/export/html_generator.cpp \
               $(QUANTA_GLIA_DIR)/src/config/config.cpp \
               $(QUANTA_GLIA_DIR)/src/storage/storage.cpp \
               $(QUANTA_GLIA_DIR)/src/curate/curator.cpp \
               $(QUANTA_GLIA_DIR)/src/annotate/annotator.cpp \
               $(QUANTA_ETHOS_DIR)/src/ethos/bias_scanner.cpp \
               $(QUANTA_ETHOS_DIR)/src/ethos/pii_redactor.cpp \
               $(QUANTA_ETHOS_DIR)/src/ethos/toxicity_filter.cpp \
               $(QUANTA_GLIA_DIR)/src/util/translator.cpp \
               $(QUANTA_HABA_DIR)/src/c/editor/EditorLogic.cpp

REAL_INT_CXXFLAGS = -std=c++17 \
                    -Isrc \
                    -I$(QUANTA_ETHOS_DIR)/src \
                    -I$(QUANTA_ETHOS_DIR)/include \
                    -I$(QUANTA_ETHOS_DIR)/include/ethos \
                    -I$(QUANTA_TISSU_DIR)/tisslm/compiler \
                    -I$(QUANTA_HABA_DIR)/src/c/export \
                    -I$(QUANTA_HABA_DIR)/src/c/core \
                    -I$(QUANTA_HABA_DIR)/src/c/editor \
                    -I$(QUANTA_GLIA_DIR)/src \
                    -I$(QUANTA_GLIA_DIR)/src/config \
                    -I$(QUANTA_GLIA_DIR)/src/util \
                    -I$(QUANTA_GLIA_DIR)/src/annotate \
                    -Wall -Wextra

real_integration_tests: $(REAL_INT_SRC)
	$(CXX) $(REAL_INT_CXXFLAGS) $(REAL_INT_SRC) -o run_real_integration_tests
	./run_real_integration_tests

clean:
	rm -f quantalista run_tests run_bridge_tests run_e2e_tests

ENHANCED_INT_SRC = test/integration/enhanced_integration_tests.cpp \
                   src/core/core.cpp \
                   src/models/ModelBackend.cpp \
                   src/utils/json_utils.cpp \
                   src/events/events.cpp \
                   $(QUANTA_ETHOS_DIR)/src/QuantaEthos/ethics_logic.cpp \
                   $(QUANTA_TISSU_DIR)/tisslm/compiler/parser.cpp \
                   $(QUANTA_HABA_DIR)/src/c/export/html_generator.cpp \
                   $(QUANTA_GLIA_DIR)/src/config/config.cpp \
                   $(QUANTA_GLIA_DIR)/src/storage/storage.cpp \
                   $(QUANTA_GLIA_DIR)/src/curate/curator.cpp \
                   $(QUANTA_GLIA_DIR)/src/annotate/annotator.cpp \
                   $(QUANTA_ETHOS_DIR)/src/ethos/bias_scanner.cpp \
                   $(QUANTA_ETHOS_DIR)/src/ethos/pii_redactor.cpp \
                   $(QUANTA_ETHOS_DIR)/src/ethos/toxicity_filter.cpp \
                   $(QUANTA_GLIA_DIR)/src/util/translator.cpp \
                   $(QUANTA_HABA_DIR)/src/c/editor/EditorLogic.cpp

enhanced_integration_tests: $(ENHANCED_INT_SRC)
	$(CXX) $(REAL_INT_CXXFLAGS) $(ENHANCED_INT_SRC) -o run_enhanced_integration_tests
	./run_enhanced_integration_tests
