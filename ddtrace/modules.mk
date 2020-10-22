mod_ddtrace.la: mod_ddtrace.slo ddtrace.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_ddtrace.lo ddtrace.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_ddtrace.la
