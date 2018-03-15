# Copyright © 2018  Max Wällstedt <max.wallstedt@gmail.com>
#
# This file is part of maxserver.
#
# maxserver is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# maxserver is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with maxserver.  If not, see
# <http://www.gnu.org/licenses/>.

RM = rm -f
CP = cp
MKDIR = mkdir -p
INSTALL = install
LN = ln -sf
PREFIX = $(HOME)
SRCDIR = src
EXAMPLESDIR = examples
TARGET = maxserver

all: lib$(TARGET).so.1.0 $(TARGET).h

lib$(TARGET).so.1.0: $(SRCDIR)/lib$(TARGET).so.1.0
	@echo -e "CP\t$< $@"
	@$(CP) $< $@

$(TARGET).h: $(SRCDIR)/$(TARGET).h
	@echo -e "CP\t$< $@"
	@$(CP) $< $@

install: \
	$(PREFIX)/lib/lib$(TARGET).so.1.0 \
	$(PREFIX)/lib/lib$(TARGET).so.1 \
	$(PREFIX)/lib/lib$(TARGET).so \
	$(PREFIX)/include/$(TARGET).h

$(PREFIX)/lib/lib$(TARGET).so.1.0: \
	lib$(TARGET).so.1.0 | \
	$(PREFIX)/lib
	@echo -e "INSTALL\t$@"
	@$(INSTALL) $< $@

$(PREFIX)/lib/lib$(TARGET).so.1: \
	$(PREFIX)/lib/lib$(TARGET).so.1.0
	@echo -e "LN\t$@"
	@$(LN) $< $@

$(PREFIX)/lib/lib$(TARGET).so: \
	$(PREFIX)/lib/lib$(TARGET).so.1.0
	@echo -e "LN\t$@"
	@$(LN) $< $@

$(PREFIX)/include/$(TARGET).h: \
	$(TARGET).h | \
	$(PREFIX)/include
	@echo -e "INSTALL\t$@"
	@$(INSTALL) $< $@

$(PREFIX)/lib:
	@echo -e "MKDIR\t$@"
	@$(MKDIR) $@

$(PREFIX)/include:
	@echo -e "MKDIR\t$@"
	@$(MKDIR) $@

.PHONY: $(SRCDIR)/lib$(TARGET).1.0 examples uninstall clean distclean

$(SRCDIR)/lib$(TARGET).so.1.0:
	@echo -e "MAKE\t$(SRCDIR)/"
	@$(MAKE) -C $(SRCDIR)

examples:
	@echo -e "MAKE\t$(EXAMPLESDIR)/"
	@$(MAKE) -C $(EXAMPLESDIR)

uninstall:
	@echo -e "RM\t$(PREFIX)/lib/lib$(TARGET).so.1.0"
	@$(RM) $(PREFIX)/lib/lib$(TARGET).so.1.0
	@echo -e "RM\t$(PREFIX)/lib/lib$(TARGET).so.1"
	@$(RM) $(PREFIX)/lib/lib$(TARGET).so.1
	@echo -e "RM\t$(PREFIX)/lib/lib$(TARGET).so"
	@$(RM) $(PREFIX)/lib/lib$(TARGET).so
	@echo -e "RM\t$(PREFIX)/include/$(TARGET).h"
	@$(RM) $(PREFIX)/include/$(TARGET).h

clean:
	@echo -e "RM\tlib$(TARGET).so.1.0"
	@$(RM) lib$(TARGET).so.1.0
	@echo -e "RM\t$(TARGET).h"
	@$(RM) $(TARGET).h
	@echo -e "MAKE\t$(SRCDIR)/ clean"
	@$(MAKE) -C $(SRCDIR) clean

distclean: clean
	@echo -e "MAKE\t$(EXAMPLESDIR)/ clean"
	@$(MAKE) -C $(EXAMPLESDIR) clean
