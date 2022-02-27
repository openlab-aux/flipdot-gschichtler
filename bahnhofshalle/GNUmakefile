DIST ?= dist
INSTALL ?= install

ESBUILDFLAGS =  --minify --bundle --platform=browser
ESBUILDFLAGS += --target=es6,edge14,firefox51,chrome49,safari11

DISTFILES =  $(DIST)/index.html
DISTFILES += $(DIST)/favicon.ico
DISTFILES += $(DIST)/openlab-logo.png
DISTFILES += $(DIST)/style.css
DISTFILES += $(DIST)/main.js

all: main.js

dist: $(DISTFILES)

$(DIST)/%.css: %.css
	esbuild $(ESBUILDFLAGS) $< --outfile=$@

$(DIST)/%: %
	$(INSTALL) -Dm644 $< -t $(DIST)

%.js: %.es6
	esbuild $(ESBUILDFLAGS) --loader=js --outfile=$@ < $<

clean:
	rm -f *.js
	rm -rf $(DIST)

.PHONY: clean
