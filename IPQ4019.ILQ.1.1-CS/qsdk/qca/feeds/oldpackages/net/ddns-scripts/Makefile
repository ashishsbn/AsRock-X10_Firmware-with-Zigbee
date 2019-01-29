include $(TOPDIR)/rules.mk

PKG_NAME:=ddns-scripts
PKG_VERSION:=1.0.0
PKG_RELEASE:=22

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/ddns-scripts
	SECTION:=net
	CATEGORY:=Network
        SUBMENU:=IP Addresses and Names
	TITLE:=Dynamic DNS Scripts
	PKGARCH:=all
endef

define Package/ddns-scripts/description
	A highly configurable set of scripts for doing
	dynamic dns updates
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/ddns-scripts/conffiles
/etc/config/ddns
endef

define Package/ddns-scripts/install
	$(INSTALL_DIR) $(1)
	$(CP) ./files/* $(1)/
ifeq ($(CONFIG_TARGET_ipq806x_QSDK_ASRock_X10),y)
	$(RM) $(1)/usr/lib/ddns/dynamic_dns_functions.sh
	$(CP) ./files/usr/lib/ddns/dynamic_dns_functions-X10.sh $(1)/usr/lib/ddns/dynamic_dns_functions.sh
	$(RM) $(1)/usr/lib/ddns/dynamic_dns_functions-X10.sh
	$(RM) $(1)/usr/lib/ddns/dynamic_dns_updater.sh
	$(CP) ./files/usr/lib/ddns/dynamic_dns_updater-X10.sh $(1)/usr/lib/ddns/dynamic_dns_updater.sh
	$(RM) $(1)/usr/lib/ddns/dynamic_dns_updater-X10.sh
	$(RM) $(1)/usr/lib/ddns/services
	$(CP) ./files/usr/lib/ddns/services-X10 $(1)/usr/lib/ddns/services
	$(RM) $(1)/usr/lib/ddns/services-X10

	$(RM) $(1)/etc/config/ddns
	$(CP) ./files/etc/config/ddns-X10 $(1)/etc/config/ddns
	$(RM) $(1)/etc/config/ddns-X10
endif

	$(RM) $(1)/usr/lib/ddns/url_escape.sed
	$(CP) ./files/usr/lib/ddns/url_escape-X10.sed $(1)/usr/lib/ddns/url_escape.sed
	$(RM) $(1)/usr/lib/ddns/url_escape-X10.sed

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/ddns $(1)/etc/init.d/
endef

$(eval $(call BuildPackage,ddns-scripts))
