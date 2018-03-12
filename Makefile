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

TARGET = maxserver
PREFIX = $(HOME)
SRCDIR = src

INSTALL = install
MKDIR = mkdir
CP = cp
RM = rm -f
LN = ln -sf

all: lib$(TARGET).so.1.0 $(TARGET).h

lib$(TARGET).so.1.0: $(SRCDIR)/lib$(TARGET).so.1.0
	@echo -e "CP\t$< $@"
	@$(CP) $< $@

$(TARGET).h: $(SRCDIR)/$(TARGET).h
	@echo -e "CP\t$< $@"
	@$(CP) $< $@

$(PREFIX)/lib:
	@echo -e "MKDIR\t$@"
	@$(MKDIR) $@

$(PREFIX)/include:
	@echo -e "MKDIR\t$@"
	@$(MKDIR) $@

install: \
	lib$(TARGET).so.1.0 \
	$(TARGET).h \
	$(PREFIX)/lib \
	$(PREFIX)/include
	@echo -e "INSTALL\t$(PREFIX)/lib/lib$(TARGET).so.1.0"
	@$(INSTALL) lib$(TARGET).so.1.0 $(PREFIX)/lib/lib$(TARGET).so.1.0
	@echo -e "LN\t$(PREFIX)/lib/lib$(TARGET).so.1"
	@$(LN) $(PREFIX)/lib/lib$(TARGET).so.1.0 $(PREFIX)/lib/lib$(TARGET).so.1
	@echo -e "LN\t$(PREFIX)/lib/lib$(TARGET).so"
	@$(LN) $(PREFIX)/lib/lib$(TARGET).so.1.0 $(PREFIX)/lib/lib$(TARGET).so
	@echo -e "INSTALL\t$(PREFIX)/include/$(TARGET).h"
	@$(INSTALL) $(TARGET).h $(PREFIX)/include/$(TARGET).h

.PHONY: $(SRCDIR)/lib$(TARGET).1.0 uninstall clean

$(SRCDIR)/lib$(TARGET).so.1.0:
	@echo -e "MAKE\t$(SRCDIR)"
	@$(MAKE) -C $(SRCDIR)

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
	@echo -e "MAKE\t$(SRCDIR) clean"
	@$(MAKE) -C $(SRCDIR) clean
	@echo -e "RM\tlib$(TARGET).so.1.0"
	@$(RM) lib$(TARGET).so.1.0
	@echo -e "RM\t$(TARGET).h"
	@$(RM) $(TARGET).h
