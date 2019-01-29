/* asrock-ap.js -- specific javaScript for ASRock X10 AP mode */
function writeRpLeftMenu()
{
  document.write("<div id=\"nav\">");
  document.write("  <ul class\"menus\">");
  document.write("    <li class=\"menu\" ><span><script>document.write(AdvWireless);</script></span>");
  document.write("      <ul id=\"wirelessMenu\">");
  document.write("        <li class=\"submenu\" id=\"wireless2G\"><span><script>document.write(AdvExt24G);</script></span><a href=\"adv_wlan_2g.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wireless5G\"><span><script>document.write(AdvExt5G);</script></span><a href=\"adv_wlan_5g.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wirelessWps\"><span><script>document.write(AdvWPS);</script></span><a href=\"adv_wlan_wps.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wirelessRescan\"><span><script>document.write(AdvRescan);</script></span><a href=\"adv_wlan_rescan.html\"></a></li>");
  document.write("      </ul>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" id=\"lanMenu\"><span><script>document.write(AdvLAN);</script></span><a href=\"adv_lan.html\"></a>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" id=\"usbMenu\"><span align=\"left\"><script>document.write(DashUSB);</script></span><a href=\"adv_USB.html\"></a>");
  document.write("    </li>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" id=\"homeMenu\"><span align=\"left\"><script>document.write(DashSmartHome);</script></span><a href=\"adv_home_sensors.html\"></a>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" ><span align=\"left\"><script>document.write(AdvAdministration);</script></span>");
  document.write("      <ul id=\"administrationMenu\">");
  document.write("        <li class=\"submenu\" id=\"adminPanel\"><span><script>document.write(AdvSystemPanel);</script></span><a href=\"adv_system_panel.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"adminOpMode\"><span><script>document.write(AdvWirelessOMode);</script></span><a href=\"adv_op_mode.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"adminChangePassword\"><span><script>document.write(AdvChangePassword);</script></span><a href=\"adv_change_password.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"adminFwUpgrade\"><span><script>document.write(AdvFWUpgrade);</script></span><a href=\"adv_firmwareUpgrade.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"adminSelfHealing\"><span><script>document.write(AdvSelfHealing);</script></span><a href=\"adv_selfHealing.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"adminRestoreSave\"><span><script>document.write(AdvRestoreSave);</script></span><a href=\"adv_restoreSave.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"adminSysLog\"><span><script>document.write(AdvSysLog);</script></span><a href=\"adv_systemLog.html\"></a></li>");
  document.write("      </ul>");
  document.write("    </li>");
  document.write("  </ul>");
  document.write("</div>");
}
