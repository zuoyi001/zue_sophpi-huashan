
all:
	@echo "##############################"
	@echo "#                            #"
	@echo "#   Compiling 'samples'...   #"
	@echo "#                            #"
	@echo "##############################"
	@cd common; make all || exit 1; cd ../;
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; \
	    do cd `dirname $$x`; if [ $$? ]; then $(MAKE) all || exit 1; cd ../; fi; done
	make -C audio

clean:
	@for x in `find ./ -maxdepth 2 -mindepth 2 -name "Makefile" `; \
	    do cd `dirname $$x`; if [ $$? ]; then $(MAKE) clean; cd ../; fi; done
	@if [ -d "./audio" ]; then \
	echo "sample/audio eixst"; cd ./audio; make clean; cd ..; \
	else \
	echo "sample/audio not exist!!"; \
	fi