include $(TOPDIR)/rules.mk

# Name, version and release number
# The name and version of your package are used to define the variable to point to the build directory of your package: $(PKG_BUILD_DIR)
PKG_NAME:=x10_ubus
PKG_VERSION:=1.0
PKG_RELEASE:=1

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
SOURCE_DIR:=/home/buildbot/x10_ubus
LIB_FLAG = -lubox -lubus -lblobmsg_json -lrt -lpthread -lsqlite3 -lxml2 -lcurl
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libxml2
include $(INCLUDE_DIR)/package.mk

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
define Package/x10_ubus
  SECTION:=Kochar
  CATEGORY:=Kochar
  TITLE:=IOT_Module
  DEPENDS:=+libubox +libubus +libblobmsg-json +librt +libpthread +libsqlite3 +libxml2 +libcurl
endef

# Package description; a more verbose description on what our package does
define Package/x10_ubus/description
  A simple "Hello, world!" -application.
endef

# Package preparation instructions; create the build directory and copy the source code. 
# The last command is necessary to ensure our preparation instructions remain compatible with the patching system.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp ./src/* $(PKG_BUILD_DIR)
	cp ./files/* $(PKG_BUILD_DIR)
endef

# Package build instructions; invoke the target-specific compiler to first compile the source file, and then to link the file into the final executable
define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/zstack_ubus.o -c $(PKG_BUILD_DIR)/zstack_ubus.c
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/cloud_client.o -c $(PKG_BUILD_DIR)/cloud_client.c
	$(TARGET_CC) $(TARGET_LDFLAGS) $(LIB_FLAG) -o $(PKG_BUILD_DIR)/zstack_ubus $(PKG_BUILD_DIR)/zstack_ubus.o
	$(TARGET_CC) $(TARGET_LDFLAGS) $(LIB_FLAG) -o $(PKG_BUILD_DIR)/cloud_client $(PKG_BUILD_DIR)/cloud_client.o
endef

define Package/x10_ubus/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/gizmo/action
	$(INSTALL_DIR) $(1)/etc/gizmo/cronjobs
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/device*  $(1)/etc/gizmo/action
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/add_rule  $(1)/etc/gizmo
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/deviceLrStatus  $(1)/etc/gizmo
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/welcomeGet  $(1)/etc/gizmo
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/add_group  $(1)/etc/gizmo
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/add_schedule  $(1)/etc/gizmo
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scene_triggered $(1)/etc/gizmo
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/call_group $(1)/etc/gizmo
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/delete_gateway  $(1)/etc/gizmo
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cloud_client.init  $(1)/etc/init.d/cloud_client
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zstack_ubus.init  $(1)/etc/init.d/zstack_ubus
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zstack_ubus $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cloud_client $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fwUpgrade.sh $(1)/etc/gizmo
endef

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,x10_ubus))
