
include $(TOPDIR)/rules.mk

PKG:=qca-hostap
PKG_NAME:=$(PKG)-10.4
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=ssh://qca-git01.qualcomm.com:29418/wifi/$(PKG_NAME).git
PKG_BRANCH:=master
PKG_RELEASE:=1

include $(INCLUDE_DIR)/local-development.mk
ifeq ($(DUMP)$(PKG_VERSION),)
  PKG_REV:=$(shell git ls-remote $(PKG_SOURCE_URL) $(PKG_BRANCH) | cut -b -7)
  PKG_VERSION:=g$(PKG_REV)
endif

LOCAL_TYPE=$(strip \
		$(if $(findstring supplicant,$(BUILD_VARIANT)),supplicant, \
		hostap \
	))

LOCAL_VARIANT=$(patsubst supplicant-%,%,$(BUILD_VARIANT))

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG)-$(PKG_VERSION)

WIFI_10_4_U:=PACKAGE_kmod-qca-wifi-10.4-unified-profile
WIFI_10_4_A:=PACKAGE_kmod-qca-wifi-10.4-akronite-perf
WIFI_10_4_D:=PACKAGE_kmod-qca-wifi-10.4-dakota-perf
WIFI_10_4_MIPS:=PACKAGE_kmod-qca-wifi-10.4-perf
WIFI_10_2_MIPS:=PACKAGE_kmod-qca-wifi-perf
WIFI_10_2_ARM:=PACKAGE_kmod-qca-wifi-akronite-perf
WIFI_10_4_MIPS_UNIFIED:=PACKAGE_kmod-qca-wifi-10.4-unified-perf
WIFI_PKGS:=$(WIFI_10_4_U)||$(WIFI_10_4_A)||$(WIFI_10_4_D)||$(WIFI_10_4_MIPS)||$(WIFI_10_2_MIPS)||$(WIFI_10_2_ARM)||$(WIFI_10_4_MIPS_UNIFIED)
PKG_BUILD_DEPENDS:=$(WIFI_10_4_U):qca-wifi-10.4 $(WIFI_10_4_A):qca-wifi-10.4 $(WIFI_10_4_D):qca-wifi-10.4 \
	$(WIFI_10_2_MIPS):qca-wifi $(WIFI_10_2_ARM):qca-wifi $(WIFI_10_4_MIPS):qca-wifi-10.4 \
	$(WIFI_10_4_MIPS_UNIFIED):qca-wifi-10.4

include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=QCA
  CATEGORY:=QCA Proprietary software
  URL:=http://www.qca.qualcomm.com
  MAINTAINER:=Qualcomm Atheros
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=Hostap authenticator
  DEPENDS:=@$(WIFI_PKGS) @(!(PACKAGE_hostapd)) +libnl-bf
  VARIANT:=default
endef

define Package/$(PKG_NAME)/description
 This package contains the IEEE 802.1x/WPA/EAP/RADIUS Authenticator.
endef

define Package/qca-hostapd-cli-10.4
  $(call Package/$(PKG_NAME)/Default)
  DEPENDS:=$(PKG_NAME) +librt
  TITLE:=Hostap authenticator command line interface
endef

define Package/qca-hostapd-cli-10.4/description
 This package contains a command line utility to control the
 IEEE 802.1x/WPA/EAP/RADIUS Authenticator.
endef

define Package/qca-wpa-supplicant-10.4
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=WPA Supplicant
  DEPENDS:=@$(WIFI_PKGS) @(!(PACKAGE_wpa-supplicant)) +libopenssl +libnl-bf
  VARIANT:=supplicant-default
endef

define Package/qca-wpa-supplicant-10.4/Description
  WPA Supplicant
endef

define Package/qca-wpa-supplicant-10.4-macsec
  $(call Package/qca-wpa-supplicant-10.4)
  TITLE:=WPA Supplicant (MACSEC)
  DEPENDS+= @TARGET_ipq806x +DEFAULT_kmod-qca-nss-macsec:kmod-qca-nss-macsec
  VARIANT:=supplicant-macsec
endef

define Package/qca-wpa-supplicant-10.4-macsec/Description
  WPA Supplicant with MACSEC support for IPQ8066.
endef

define Package/qca-wpa-cli-10.4
  $(call Package/$(PKG_NAME)/Default)
  DEPENDS:=@PACKAGE_qca-wpa-supplicant-10.4||PACKAGE_qca-wpa-supplicant-10.4-macsec +librt
  TITLE:=WPA Supplicant command line interface
endef

define Package/qca-wpa-cli-10.4/Description
  WPA Supplicant control utility
endef


ifneq ($(wildcard $(PKG_BUILD_DIR)/.config_*),$(subst .configured_,.config_,$(STAMP_CONFIGURED)))
  define Build/Configure/rebuild
	$(FIND) $(PKG_BUILD_DIR) -name \*.o -or -name \*.a | $(XARGS) rm -f
	rm -f $(PKG_BUILD_DIR)/hostapd/hostapd
	rm -f $(PKG_BUILD_DIR)/wpa_supplicant/wpa_supplicant
	rm -f $(PKG_BUILD_DIR)/.config_*
	touch $(subst .configured_,.config_,$(STAMP_CONFIGURED))
  endef
endif


define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_source_path,$(PKG_NAME))
$(call Package/$(PKG_NAME)/override_version,$(PKG_NAME),$(PKG_SUPPORTED_VERSION))
endef

define Build/Configure
	$(Build/Configure/rebuild)
	$(if $(wildcard ./files/hostapd-$(LOCAL_VARIANT).config), \
		$(CP) ./files/hostapd-$(LOCAL_VARIANT).config $(PKG_BUILD_DIR)/hostapd/.config \
	)
	$(CP) ./files/wpa_supplicant-10.4-$(LOCAL_VARIANT).config $(PKG_BUILD_DIR)/wpa_supplicant/.config
endef

TARGET_CPPFLAGS += \
	-I$(STAGING_DIR)/usr/include \
	-I$(STAGING_DIR)/usr/include/qca-nss-macsec

TARGET_LDFLAGS += \
	-lnl-3 -lnl-genl-3 -lm

ifneq ($(LOCAL_TYPE),hostap)
    TARGET_LDFLAGS += \
		-lssl -lcrypto
  ifeq ($(LOCAL_VARIANT),macsec)
    TARGET_LDFLAGS += -lfal -L$(STAGING_DIR)/usr/lib
  endif
endif

define Build/RunMake
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	$(MAKE) -C $(PKG_BUILD_DIR)/$(1) \
	$(TARGET_CONFIGURE_OPTS) \
	LIBS="$(TARGET_LDFLAGS)" \
	$(2)
endef

define Build/Compile/qca-hostap
	$(call Build/RunMake,hostapd, \
		hostapd hostapd_cli \
	)
endef

define Build/Compile/qca-supplicant
	$(call Build/RunMake,wpa_supplicant, \
		wpa_supplicant wpa_cli \
	)
endef

define Build/Compile
	$(Build/Compile/qca-$(LOCAL_TYPE))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/common/wpa_ctrl.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/common/ieee802_11_defs.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/includes.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/build_config.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/eloop.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/os.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/common.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/wpa_debug.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/utils/wpabuf.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/drivers/linux_ioctl.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/drivers/linux_wext.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/drivers/netlink.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/drivers/priv_netlink.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/common/wpa_ctrl.o $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/drivers/netlink.o $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/utils/eloop.o $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/utils/wpa_debug.o $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/utils/common.o $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/utils/os_unix.o $(1)/usr/lib/
endef

define Install/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_DATA) ./files/hostapd.sh $(1)/lib/wifi/hostapd.sh
	$(INSTALL_DIR) $(1)/usr/sbin
endef

define Install/qca-wpa-supplicant-10.4
	$(INSTALL_DIR) $(1)/lib/wifi
ifeq ($(CONFIG_TARGET_ipq806x_QSDK_ASRock_X10),y)
	$(INSTALL_DATA) ./files/x10-wpa_supplicant.sh $(1)/lib/wifi/wpa_supplicant.sh
else
	$(INSTALL_DATA) ./files/wpa_supplicant.sh $(1)/lib/wifi/wpa_supplicant.sh
endif
	$(INSTALL_DIR) $(1)/usr/sbin
endef

define Package/$(PKG_NAME)/install
	$(call Install/$(PKG_NAME),$(1))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd $(1)/usr/sbin/
endef

ifneq ($(LOCAL_TYPE),supplicant)
  define Package/qca-hostapd-cli-10.4/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/hotplug.d/button
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd_cli $(1)/usr/sbin/
	$(INSTALL_DATA) ./files/wps-hotplug.sh $(1)/etc/hotplug.d/button/50-wps
	$(INSTALL_DATA) ./files/wps-extender-hotplug.sh \
			$(1)/etc/hotplug.d/button/54-wps-extender
	$(INSTALL_DATA) ./files/wps-reset-hotplug.sh $(1)/etc/hotplug.d/button/51-wps-reset

	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_BIN) ./files/wps-hostapd-update-uci.sh \
			$(1)/lib/wifi/wps-hostapd-update-uci

ifeq ($(CONFIG_TARGET_ipq806x_QSDK_ASRock_X10),y)
	$(INSTALL_DATA) ./files/x10-wps-hotplug.sh $(1)/etc/hotplug.d/button/50-wps
	rm $(1)/etc/hotplug.d/button/51-wps-reset
	$(INSTALL_BIN) ./files/x10-wps-hostapd-update-uci.sh \
			$(1)/lib/wifi/wps-hostapd-update-uci
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./files/x10-wps-update-status.sh \
			$(1)/etc/hotplug.d/iface/50-wps-status
endif
  endef
endif

define Package/qca-wpa-supplicant-10.4/install
	$(call Install/qca-wpa-supplicant-10.4,$(1))
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wpa_supplicant/wpa_supplicant $(1)/usr/sbin/
endef

define Package/qca-wpa-supplicant-10.4-macsec/install
	$(call Package/qca-wpa-supplicant-10.4/install,$(1))
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) ./files/macsec.init $(1)/etc/init.d/macsec
	$(INSTALL_BIN) ./files/macsec.config $(1)/etc/config/macsec
endef

ifneq ($(LOCAL_TYPE),hostap)
  define Package/qca-wpa-cli-10.4/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/wpa_supplicant/wpa_cli $(1)/usr/sbin/

	$(INSTALL_DIR) $(1)/etc/hotplug.d/button
	$(INSTALL_DATA) ./files/wps-supplicant-hotplug.sh \
			$(1)/etc/hotplug.d/button/52-wps-supplicant

	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_BIN) ./files/wps-supplicant-update-uci.sh \
			$(1)/lib/wifi/wps-supplicant-update-uci

ifeq ($(CONFIG_TARGET_ipq806x_QSDK_ASRock_X10),y)
	$(INSTALL_DATA) ./files/x10-wps-supplicant-hotplug.sh \
			$(1)/etc/hotplug.d/button/52-wps-supplicant
	$(INSTALL_BIN) ./files/x10-wps-supplicant-update-uci.sh \
			$(1)/lib/wifi/wps-supplicant-update-uci
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/x10-wps_sta.sh \
			$(1)/usr/sbin/wps_sta
endif
  endef
endif

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,qca-wpa-supplicant-10.4))
$(eval $(call BuildPackage,qca-wpa-supplicant-10.4-macsec))
$(eval $(call BuildPackage,qca-hostapd-cli-10.4))
$(eval $(call BuildPackage,qca-wpa-cli-10.4))
