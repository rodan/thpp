
all: tags

tags: force
	@find ./ -type f \( -iname \*.cpp -o -iname \*.h \)  | grep -v './misc' | exuberant-ctags -L -

force:
	@true
