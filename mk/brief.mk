##
# `brief.mk' - brief
#
# copyright (c) 2014 - joseph werle <joseph.werle@gmail.com>
##

##
# BRIEF output format
##

BRIEF_FORMAT ?= "  %s   \t%s\n"

##
# BRIEF output routine
##

BRIEF_ECHO ?= printf $(BRIEF_FORMAT) "$(1)" "$(2)" | tr '\n' ' ' && echo

##
# BRIEF argument expansion
##

BRIEF_ARGS ?= $@

##
# Expand each command command found
# in `BRIEFC' into a routine
##

ifdef BRIEFC
ifeq (1,$(BRIEF_NO_ESCAPE))
$(foreach C, $(BRIEFC), \
	    $(eval $(C) = $$(call BRIEF_ECHO, $(C), $$(BRIEF_ARGS)); $($(C))))
else
$(foreach C, $(BRIEFC), \
	    $(eval $(C) = @$$(call BRIEF_ECHO, $(C), $$(BRIEF_ARGS)); $($(C))))
endif
endif

