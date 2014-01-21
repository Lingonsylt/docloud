.PHONY: all clean
-include config.mk

all:
	@$(MAKE) $(MFLAGS) $(MAKECMDGOALS) -C shared
	@$(MAKE) $(MFLAGS) $(MAKECMDGOALS) -C docloudext
	@$(MAKE) $(MFLAGS) $(MAKECMDGOALS) -C service
	@$(MAKE) $(MFLAGS) $(MAKECMDGOALS) -C settings
clean:
	@$(MAKE) $(MFLAGS) $(MAKECMDGOALS) -C shared
	@$(MAKE) $(MFLAGS) $(MAKECMDGOALS) -C docloudext
	@$(MAKE) $(MFLAGS) $(MAKECMDGOALS) -C service
	@$(MAKE) $(MFLAGS) $(MAKECMDGOALS) -C settings

