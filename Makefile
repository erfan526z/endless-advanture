BUILD_DIR = build
TARGET = EndlessAdventure

.PHONY: all debug release clean

all:
	@echo "Use: make debug or make release"

debug:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j$(nproc)

release:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(nproc)

clean:
	rm -rf $(BUILD_DIR)

