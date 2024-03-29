#
# Copyright (C) 2011-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=perf
PKG_VERSION:=$(LINUX_VERSION)
PKG_RELEASE:=2

PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/perf
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:= +libelf1 +libdw +libpthread +librt +binutils
  TITLE:=Linux performance monitoring tool
  VERSION:=$(LINUX_VERSION)-$(PKG_RELEASE)
  URL:=http://www.kernel.org
  KCONFIG:=CONFIG_PERF_EVENTS=y \
	CONFIG_HW_PERF_EVENTS=y
endef

define Package/perf/description
  perf is the Linux performance monitoring tool
endef

# Perf's makefile and headers are not relocatable and must be built from the
# Linux sources directory
define Build/Prepare
	$(RM) -r $(PKG_BUILD_DIR)
	ln -s $(LINUX_DIR)/tools/perf $(PKG_BUILD_DIR)
endef

# MAKE_FLAGS should be passed again upon cleaning because Perf's makefile
# always performs checks before processing any rule
define Build/Clean
	-$(MAKE) -C $(PKG_BUILD_DIR) \
		NO_DWARF=1 \
		$(MAKE_FLAGS) \
		clean
endef

MAKE_FLAGS = \
	ARCH="$(LINUX_KARCH)" \
	NO_LIBPERL=1 \
	NO_LIBPYTHON=1 \
	NO_NEWT=1 \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	CC="$(TARGET_CC)" \
	LD="$(TARGET_CROSS)ld" \
	CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	V=1 \
	prefix=/usr \
	WERROR=0

define Package/perf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(LINUX_DIR)/tools/perf/perf $(1)/usr/bin/
endef

$(eval $(call BuildPackage,perf))
