# Nome dell'eseguibile
TARGET = SerialXNET

# Compilatore
CXX = g++

# Flag di compilazione
CXXFLAGS = -Wall -std=c++11 -Isrc -Inet

# Cartelle
SRC_DIRS = src src/net
OBJ_DIR = obj

# File sorgente da tutte le cartelle in SRC_DIRS
SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
OBJECTS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

# Regola principale
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET)

# Compilazione oggetti (mantiene la struttura di sottocartelle in obj/)
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean