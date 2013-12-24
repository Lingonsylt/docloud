.PHONY: all clean

all:
	@$(MAKE) $(MFLAGS) -C shared
	@$(MAKE) $(MFLAGS) -C docloudext
	@$(MAKE) $(MFLAGS) -C service
clean:
	@$(MAKE) $(MFLAGS) clean -C shared
	@$(MAKE) $(MFLAGS) clean -C docloudext
	@$(MAKE) $(MFLAGS) clean -C service

-include config.mk
