/* dashboard-br.js -- specific javascript for ASRock dashboard page on bridge mode*/
function writeDashboardBrLanWindow(id)
{
  document.write('\
    <div id="' + id + '" style="display:none">\
      <table width="502" border="0" cellspacing="0" cellpadding="0">\
        <tr>\
          <td background="images/box_01.png" width="502" height="46">\
            <table width="100%" border="0" cellspacing="0" cellpadding="0">\
              <tr>\
                <td width="92%" align="left"><p>' + DashApStatus + '</p></td>\
                <td width="8%" align="left">\
                  <a onclick="closeWindow(\'' + id + '\')" href="#" onmouseout="MM_swapImage(\'Image17\',\'\',\'images/botton05-nol.png\',1)" onmouseover="MM_swapImage(\'Image17\',\'\',\'images/botton05-over.png\',1)">\
                    <img src="images/botton05-nol.png" name="Image17" width="26" height="26" border="0" id="Image17" />\
                  </a>\
                </td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td background="images/box_02.png" height="46"></td>\
        </tr>\
        <tr>\
          <td align="center" background="images/box_03.png">\
            <table width="96%" border="0" cellspacing="3" cellpadding="3">' +
              dashboardWindowFont("rootApSsid", DashApStatus) +
              dashboardWindowFont("lanType", DashConnectionType) +
              dashboardWindowFont("rootApBand", RpWlanBand) +
              dashboardWindowFont("rootApLinkRate", DashLinkRate) +
              dashboardWindowImg("rootApLinkQuality", RpWlanRadioSignal) +
              '<tr>\
                <td align="left" class="style8">&nbsp;</td>\
                <td>&nbsp;</td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td colspan="2" align="center" background="images/box_03.png">\
            <table border="0" cellspacing="0" cellpadding="0">\
              <tr>\
                <td><table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                  <tr>\
                    <td class="button1" onclick="goToRescan()"><a>' + AdvRescan + '</a></td>\
                  </tr>\
                </table></td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td><img src="images/box_04.png" width="502" height="28" /></td>\
        </tr>\
      </table>\
    </div>');
}

function writeDashboardBrWindow(id)
{
  document.write('\
    <div id="' + id + '" style="display:none">\
      <table width="502" border="0" cellspacing="0" cellpadding="0">\
        <tr>\
          <td background="images/box_01.png" width="502" height="46">\
            <table width="100%" border="0" cellspacing="0" cellpadding="0">\
              <tr>\
                <td width="92%" align="left"><p>' + AdvSystemInfo + '</p></td>\
                <td width="8%" align="left">\
                  <a onclick="closeWindow(\'' + id + '\')" href="#" onmouseout="MM_swapImage(\'Image18\',\'\',\'images/botton05-nol.png\',1)" onmouseover="MM_swapImage(\'Image18\',\'\',\'images/botton05-over.png\',1)">\
                    <img src="images/botton05-nol.png" name="Image18" width="26" height="26" border="0" id="Image18" />\
                  </a>\
                </td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td width="502" height="46" align="left" background="images/box_02.png" class="style6">\
            <span>' + DashInfo + '</span>&nbsp;&nbsp;\
            <a href="adv_lan.html">\
              <span>' + GeneralEdit + '</span>\
            </a>\
          </td>\
        </tr>\
		<tr>\
          <td align="center" background="images/box_03.png">\
            <table width="96%" border="0" cellspacing="3" cellpadding="3">' + 
              dashboardWindowFont("lanIpAddr", DashLANIP) + 
              dashboardWindowFont("lanMacAddr", DashLANMACAddr) + 
              '<tr>\
                <td colspan="2" align="center" class="style8"><img src="images/line-s.png" width="470" height="5" /></td>\
              </tr>\
              <tr>\
                <td align="left" class="style8">&nbsp;</td>\
                <td>&nbsp;</td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td><img src="images/box_04.png" width="502" height="28" /></td>\
        </tr>\
      </table>\
    </div>');
}

function writeDashboardClientWindow(id)
{
  document.write('\
    <div id="' + id + '" style="display:none">\
      <table width="502" border="0" cellspacing="0" cellpadding="0">\
        <tr>\
          <td background="images/box_01.png" width="502" height="46">\
            <table width="100%" border="0" cellspacing="0" cellpadding="0">\
              <tr>\
                <td width="92%" align="left"><p>' + DashClientInfo + '</p></td>\
                <td width="8%" align="left">\
                  <a onclick="closeWindow(\'' + id + '\')" href="#" onmouseout="MM_swapImage(\'Image19\',\'\',\'images/botton05-nol.png\',1)" onmouseover="MM_swapImage(\'Image19\',\'\',\'images/botton05-over.png\',1)">\
                    <img src="images/botton05-nol.png" name="Image19" width="26" height="26" border="0" id="Image19" />\
                  </a>\
                </td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td width="502" height="46" align="left" background="images/box_02.png" class="style6">' + DashWirelessDevice + '</td>\
        </tr>\
        <tr>\
          <td width="502" align="center" background="images/box_03.png">\
            <div style="overflow:scroll;height:120px;width:97%;overflow:auto">\
            <table width="96%" border="0" cellpadding="3" cellspacing="1" id="wirelessTable">\
              <tr>\
                <td class="style8">&nbsp;</td>\
                <td align="center" class="style5" bgcolor="#666666"><h5>' + DashIPAddr + '</h5></td>\
                <td align="center" class="style5" bgcolor="#666666"><h5>' + DashDeviceName + '</h5></td>\
                <td align="center" class="style5" bgcolor="#666666"><h5>' + DashMACAddr + '</h5></td>\
              </tr>\
            </table>\
            </div>\
          </td>\
		</tr>\
		<tr>\
		  <td height="10"  background="images/box_03.png">&nbsp;</td>\
		</tr>\
		<tr>\
		  <td height="46" align="left" background="images/box_02.png" class="style6">' + DashWiredDevice + '</td>\
        </tr>\
        <tr>\
          <td align="center" background="images/box_03.png">\
            <div style="overflow:scroll;height:120px;width:97%;overflow:auto">\
            <table width="96%" border="0" cellpadding="3" cellspacing="1" id="wiredTable">\
              <tr>\
                <td class="style8">&nbsp;</td>\
                <td align="center" class="style5" bgcolor="#666666"><h5>' + DashIPAddr + '</h5></td>\
                <td align="center" class="style5" bgcolor="#666666"><h5>' + DashDeviceName + '</h5></td>\
                <td align="center" class="style5" bgcolor="#666666"><h5>' + DashMACAddr + '</h5></td>\
              </tr>\
            </table>\
            </div>\
          </td>\
        </tr>\
        <tr>\
          <td align="center" background="images/box_03.png">\
            <table border="0" cellspacing="0" cellpadding="0">\
              <tr>\
              </tr>\
            </table>\
          </td>\
         </tr>\
        <tr>\
          <td><img src="images/box_04.png" width="502" height="28" /></td>\
        </tr>\
      </table>\
    </div>');
}

function goToUsbPage()
{
  window.open("http://"+window.location.host+"/br/adv_USB.html", "_self");
}

function writeDashboardUsbWindow(id)
{
  document.write('\
    <div id="' + id + '" style="display:none">\
      <table width="502" border="0" cellspacing="0" cellpadding="0">\
        <tr>\
          <td background="images/box_01.png" width="502" height="46">\
            <table width="100%" border="0" cellspacing="0" cellpadding="0">\
              <tr>\
                <td width="92%" align="left"><p>' + DashUSBUSBInfo + '</p></td>\
                <td width="8%" align="left">\
                  <a onclick="closeWindow(\'' + id + '\')" href="#" onmouseout="MM_swapImage(\'Image20\',\'\',\'images/botton05-nol.png\',1)" onmouseover="MM_swapImage(\'Image20\',\'\',\'images/botton05-over.png\',1)">\
                    <img src="images/botton05-nol.png" name="Image20" width="26" height="26" border="0" id="Image20" />\
                  </a>\
                </td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td width="502" height="23" align="left" background="images/box_02.png" class="style6">&nbsp;</td>\
        </tr>\
        <tr>\
          <td width="502" align="center" background="images/box_03.png">\
            <table width="96%" border="0" cellpadding="3" cellspacing="1" id="usbTable">\
              <tr>\
                <td align="center" class="style5" bgcolor="#666666" width="20%"><h5>' + USBDevice + '</h5></td>\
                <td align="center" class="style5" bgcolor="#666666" width="25%"><h5>' + DashUSBTotalSpace + '</h5></td>\
                <td align="center" class="style5" bgcolor="#666666" width="25%"><h5>' + DashUSBFreeSpace + '</h5></td>\
                <td align="center" class="style5" bgcolor="#666666" width="25%"><h5>' + DashUSBApplication + '</h5></td>\
                <td align="center" class="style5" bgcolor="#666666" width="5%"><h5>&nbsp;</h5></td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td height="30" background="images/box_03.png">&nbsp;</td>\
        </tr>\
		<tr>\
          <td colspan="2" align="center" background="images/box_03.png">\
            <table border="0" cellspacing="0" cellpadding="0">\
              <tr>\
                <td><table width="230" height="31" border="0" cellspacing="0" cellpadding="0">\
                  <tr>\
                    <td class="button2" onclick="removeUSB()"><a>' + DashUSBSafetyRemove + '</a></td>\
                  </tr>\
                </table></td>\
                <td>&nbsp;</td>\
                <td><table width="230" height="31" border="0" cellspacing="0" cellpadding="0">\
                  <tr>\
                    <td class="button2" onclick="goToUsbPage()"><a>' + DashAdvance + '</a></td>\
                  </tr>\
                </table></td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td align="center" background="images/box_03.png">&nbsp;</td>\
        </tr>\
        <tr>\
          <td><img src="images/box_04.png" width="502" height="14" /></td>\
        </tr>\
      </table>\
    </div>');
}
