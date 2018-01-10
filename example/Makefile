TARGETS = raf simple

.PHONY: $(TARGETS)
$(TARGETS):
	$(MAKE) -C $@

.PHONY: clean
clean:
	$(foreach T, $(TARGETS), $(MAKE) clean -C $(T);)
