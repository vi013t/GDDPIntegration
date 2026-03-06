DATA_LOCATION="$(LOCALAPPDATA)/GeometryDash/geode/mods/minemaker0430.gddp_integration"

.PHONY: all build build-with-cache build-clean clean-cache

build:
	geode build --ninja

build-clean: clean-cache build

all: build

clean-cache:
	rm -rf $(DATA_LOCATION)/*

build-with-cache:
	mkdir -p .cache/mod-data
	rm -rf .cache/mod-data/*
	cp -r $(DATA_LOCATION)/* .cache/mod-data
	$(MAKE) build
	mkdir -p $(DATA_LOCATION)
	rm -rf $(DATA_LOCATION)/*
	cp -r .cache/mod-data/* $(DATA_LOCATION)