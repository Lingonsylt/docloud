.PHONY: all clean
-include config.mk

all:
	@$(MAKE) $(MFLAGS) all -C shared
	@$(MAKE) $(MFLAGS) all -C docloudext
	@$(MAKE) $(MFLAGS) all -C service
clean:
	@$(MAKE) $(MFLAGS) clean -C shared
	@$(MAKE) $(MFLAGS) clean -C docloudext
	@$(MAKE) $(MFLAGS) clean -C service

