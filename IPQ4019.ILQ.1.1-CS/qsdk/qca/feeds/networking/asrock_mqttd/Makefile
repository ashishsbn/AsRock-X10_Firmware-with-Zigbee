include $(TOPDIR)/rules.mk

PKG_NAME:=asrock_mqttd
PKG_VERSION:=
PKG_RELEASE:=1

include $(INCLUDE_DIR)/local-development.mk
ifeq ($(DUMP)$(PKG_VERSION),)
  PKG_REV:=$(shell git ls-remote $(PKG_SOURCE_URL) $(PKG_BRANCH) | cut -b -7)
  PKG_VERSION:=g$(PKG_REV)
endif
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

# fore disable binary strip
STRIP:=/bin/true
RSTRIP:=/bin/true

define Package/asrock_mqttd
  SECTION:=ASRock
  CATEGORY:=ASRock Proprietary software
  MAINTAINER:=Jimmy Yang
  URL:=
  TITLE:=MQTT Client IoT package
  DEPENDS:=+librt +libpthread +libubox +libubus +libblobmsg-json +libsqlite3 +luci-lib-core
endef

define Package/asrock_mqttd/conffiles
/etc/config/asrock_mqttd
endef

define Package/asrock_mqttd/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/asrock_mqttd $(1)/usr/sbin
	$(INSTALL_BIN) ./files/asrock.init $(1)/etc/init.d/asrock
	$(INSTALL_CONF) ./files/asrock_mqttd.config $(1)/etc/config/asrock
endef

$(eval $(call BuildPackage,asrock_mqttd))
