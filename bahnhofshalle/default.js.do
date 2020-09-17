redo-ifchange "$2.es6"
PATH="$PATH:node_modules/.bin"

babel --minified --no-comments --presets=@babel/env "$2.es6" > "$3"
