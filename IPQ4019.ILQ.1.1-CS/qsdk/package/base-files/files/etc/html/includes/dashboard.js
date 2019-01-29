/* dashboard.js -- specific javascript for ASRock dashboard page */
function dashboardWindowSwitch(id, desc)
{
  var html;
  
  html = '<tr>';
  html += '<td width="35%" align="left" class="style8">' + desc + '</td>';
  html += '<td width="65%" align="left">';
  html += '<span id="' + id + '" onclick="changeOnOffStat(this)">';
  html += '<img id="imageOff" src="images/botton03-off.png" width="68" height="30" />';
  html += '</span>';
  html += '</td>';
  html += '</tr>';
  
  return html;
}

function dashboardWindowFont(id, desc)
{
  var fontHtml;
  
  html = '<tr>';
  html += '<td width="35%" align="left" class="style8">' + desc + '</td>';
  html += '<td width="65%" align="left" height="25px">';
  html += '<font color="#FFCC00" style="font-weight:bold" size="2">';
  html += '<span id="' + id + '"></span>';
  html += '</font>';
  html += '</td>';
  html += '</tr>';
  
  return html;
}

function dashboardWindowInput(id, desc, maxLength)
{
  var html, length;

  if (arguments.length === 3) {
    length = maxLength;
  }
  else {
    length = 32;
  }

  html = '<tr>';
  html += '<td width="35%" align="left" class="style8">' + desc + '</td>';
  html += '<td width="65%" align="left">';
  html += '<input type="text" id="' + id + '" value="" maxlength="' + length + '">';
  html += '</td>';
  html += '</tr>';
  
  return html;
}

function dashboardWindowSelect(id, desc, lists)
{
  var html;
  
  html = '<tr>';
  html += '<td align="left" class="style8">' + desc + '</td>';
  html += '<td align="left">';
  html += '<select size="1" id="' + id + '">';
  $.each(lists, function(index, value) {
    html += "<option value=\"" + index + "\">" + value + "</option>";
  });
  html += '</select>';
  html += '</td>';
  html += '</tr>';
  
  return html;
}

function dashboardWindowImg(id, desc)
{
  var html;

  html = '<tr>';
  html += '<td width="35%" align="left" class="style8">' + desc + '</td>';
  html += '<td width="65%" align="left">';
  html += '<img src="images/Wifi-1.png" id="' + id + '" width="30" height="25" />';
  html += '</td>';
  html += '</tr>';

  return html;
}

function dashboardButton(desc, id, background, icon)
{
  var table, wordStyle;

  $('#' + id).attr('width', '156');
  $('#' + id).attr('height', '90');
  $('#' + id).attr('valign', 'top');
  $('#' + id).css('background-image', 'url(images/' + background + ')');

  if (background == "botton-new01-no.png" || background == "botton-new03-no.png")
    desc = '<font color="#666666">' + desc + '<font>'

  if ($("#langSelect").val() == "jp") {
    wordStyle = "word-break: keep-all;";
  }
  else {
    wordStyle = "word-break: break-all;";
  }

  table = '<table width="95%" border="0" cellspacing="3" cellpadding="3"> \
             <tr height="5"> \
               <td colspan="3"><img src="images/line-s.png" width="150" height="5" /></td> \
             </tr> \
             <tr height="31"> \
               <td width="5">&nbsp;</td> \
               <td width="20" class="style8"><img src="images/' + icon + '" width="20" height="20" /></td> \
               <td width="125" align="left" class="style7" style="' + wordStyle + '">' + desc + '</td> \
             </tr> \
           </table>';
  $('#' + id).append(table);
}

function dashboardButtonSlider(desc, id, background, icon)
{
  var table, wordStyle;

  $('#' + id).attr('width', '165');
  $('#' + id).attr('height', '90');
  $('#' + id).attr('valign', 'top');
  $('#' + id).css('background-image', 'url(images/' + background + ')');

  if (background == "botton-new01-no.png" || background == "botton-new03-no.png")
    desc = '<font color="#666666">' + desc + '<font>'

  if ($("#langSelect").val() == "jp") {
    wordStyle = "word-break: keep-all;";
  }
  else {
    wordStyle = "word-break: break-all;";
  }

  table = '<table width="95%" border="0" cellspacing="3" cellpadding="3"> \
             <tr height="5"> \
               <td colspan="3"><img src="images/line-s.png" width="150" height="5" /></td> \
             </tr> \
             <tr height="31"> \
               <td width="5">&nbsp;</td> \
               <td width="20" class="style8"><img src="images/' + icon + '" width="20" height="20" /></td> \
               <td width="125" align="left" class="style7" style="' + wordStyle + '">' + desc + '</td> \
             </tr> \
             <tr height="2"></tr> \
             <tr> \
               <td>&nbsp;</td> \
               <td colspan="2"> \
                 <table width="45%" align="left"> \
                   <tr><td> \
                     <div id="lightingSlider" style="display:none"></div> \
                   </td></tr> \
                 </table> \
               </td> \
             </tr> \
           </table>';
  $('#' + id).append(table);
}

function writeDashboardWanWindow(id)
{
  document.write('\
    <div id="' + id + '" style="display:none">\
      <table width="502" border="0" cellspacing="0" cellpadding="0">\
        <tr>\
          <td background="images/box_01.png" width="502" height="46">\
            <table width="100%" border="0" cellspacing="0" cellpadding="0">\
              <tr>\
                <td width="92%" align="left"><p>' + DashWANinfo + '</p></td>\
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
              dashboardWindowFont("wanType", DashConnectionType) + 
              dashboardWindowFont("wanIp", DashWANIP) + 
              dashboardWindowFont("wanMac", DashWANMACAddr) + 
              dashboardWindowFont("dnsServer", DashDNSInfo) + 
              dashboardWindowFont("gwIp", DashGatewayInfo) + 
              '<tr>\
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

function writeDashboardRouterWindow(id)
{
  var authModeList = {"0" : "OPEN", "1" : "WPA2-AES", "2" : "WPA Mixed-TKIP+AES"};
  
  document.write('\
    <div id="' + id + '" style="display:none">\
      <table width="502" border="0" cellspacing="0" cellpadding="0">\
        <tr>\
          <td background="images/box_01.png" width="502" height="46">\
            <table width="100%" border="0" cellspacing="0" cellpadding="0">\
              <tr>\
                <td width="92%" align="left"><p>' + DashWirelessInfo + '</p></td>\
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
            <table width="96%" border="0" cellspacing="3" cellpadding="3">\
              <tr>\
                <td width="35%" align="left">\
                  <span>' + Dash24G + '</span>&nbsp;&nbsp;\
                    <a href="adv_wlan_2g.html">\
                      <span>' + GeneralEdit + '</span>\
                    </a>\
                </td>\
                <td width="65%" align="left">\
                  <span id="wlanEnabled2g" onclick="changeOnOffStat(this)">\
                    <img id="imageOff" src="images/botton03-off.png" width="68" height="30" />\
                  </span>\
                </td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
        <tr>\
          <td align="center" background="images/box_03.png">\
            <table width="96%" border="0" cellspacing="3" cellpadding="3">' + 
              dashboardWindowInput("wlanSsid2g", DashWirelessName) + 
              dashboardWindowSelect("wlanAuthMode2g", DashAuthMethod, authModeList) + 
              dashboardWindowInput("wlanPsk2g", DashWPAPSKKey, 63) +
              dashboardWindowFont("wlanMacAddr2g", Dash24GWirelessMACaddr) + 
              dashboardWindowFont("pinCode2g", DashPINCode) + 
              '<tr>\
                <td colspan="2" align="center" class="style8"><img src="images/line-s.png" width="470" height="5" /></td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
		<tr>\
          <td width="502" height="46" align="left" background="images/box_02.png" class="style6">\
            <table width="96%" border="0" cellspacing="3" cellpadding="3">\
              <tr>\
                <td width="35%" align="left">\
                  <span>' + Dash5G + '</span>&nbsp;&nbsp;\
                    <a href="adv_wlan_5g.html">\
                      <span>' + GeneralEdit + '</span>\
                    </a>\
                </td>\
                <td width="65%" align="left">\
                  <span id="wlanEnabled5g" onclick="changeOnOffStat(this)">\
                    <img id="imageOff" src="images/botton03-off.png" width="68" height="30" />\
                  </span>\
                </td>\
              </tr>\
            </table>\
          </td>\
        </tr>\
		<tr>\
          <td align="center" background="images/box_03.png">\
            <table width="96%" border="0" cellspacing="3" cellpadding="3">' + 
              dashboardWindowInput("wlanSsid5g", DashWirelessName) + 
              dashboardWindowSelect("wlanAuthMode5g", DashAuthMethod, authModeList) + 
              dashboardWindowInput("wlanPsk5g", DashWPAPSKKey, 63) +
              dashboardWindowFont("wlanMacAddr5g", Dash5GWirelessMACaddr) + 
              dashboardWindowFont("pinCode5g", DashPINCode) + 
              '<tr>\
                <td colspan="2" align="center" class="style8"><img src="images/line-s.png" width="470" height="5" /></td>\
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
              dashboardWindowInput("lanIpAddr", DashLANIP, 15) +
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
          <td colspan="2" align="center" background="images/box_03.png">\
            <table border="0" cellspacing="0" cellpadding="0">\
              <tr>\
                <td><table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                  <tr>\
                    <td class="button1" onclick="setRouter()"><a>' + DashApply + '</a></td>\
                  </tr>\
                </table></td>\
                <td>&nbsp;</td>\
                <td><table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                  <tr>\
                    <td class="button1" onclick="closeWindow(\'' + id + '\')"><a>' + DashCancel + '</a></td>\
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
  window.open("http://"+window.location.host+"/adv_USB.html", "_self");
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
