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

#coverage: $(addprefix cov-, $(DIR_NAMES))

coverage: 
	cp ../../source/modules.hpp .
	clang++ $(FLAGS_COVERAGE) test_main.cpp -o MABE_coverage
	./MABE_coverage
	llvm-profdata merge default.profraw -o default.profdata
	llvm-cov show MABE_coverage -instr-profile=default.profdata > coverage.txt
	python ${EMP_DIR}/third-party/force-cover/fix_coverage.py coverage.txt
	rm modules.hpp
