DIR_NAMES= analyze core evaluate orgs placement select tools Emplode 

default: test
MABE_DIR = ../..

include $(MABE_DIR)/Makefile-base.mk
FLAGS = $(FLAGS_DEBUG)

test-%:
	cd $(@:test-%=%) && make test

#test: $(addprefix test-, $(DIR_NAMES))
test: 
	cp ../../source/modules.hpp .
	$(CXX) $(FLAGS) test_main.cpp -o MABE_test
	./MABE_test
	rm modules.hpp
	

cov-%:
	cd $(@:cov-%=%) && make coverage

coverage: $(addprefix cov-, $(DIR_NAMES))
