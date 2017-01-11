mod_waf.la: mod_waf.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_waf.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_waf.la
