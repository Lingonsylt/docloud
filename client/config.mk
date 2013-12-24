
MFLAGS= --no-print-directory
%.o: %.cpp
	@echo "$(CXX) $<"
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

%.d: %.cpp
	@echo DEPS $<
	@$(SHELL) -ec '$(CXX) $(CXXFLAGS) -MM $< | \
		sed -e '"'"'s|$*\.o: |\$*\.o $@: |g'"'"' >$@'

