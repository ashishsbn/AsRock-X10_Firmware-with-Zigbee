/* wizard-rp.js -- specific javascript for ASRock X10 EZ setup wizard on repeater mode */
function writeNoWirelessWindow()
{
  document.write('\
  <div id="noWirelessWindow" style="display:none"> \
    <table width="400" height="180" border="0" cellspacing="0" cellpadding="0" bgcolor="#333333">\
      <tr>\
        <td valign=top>\
          <table width="100%" border="0" cellspacing="0" cellpadding="0">\
            <tr>\
              <td width="92%" align="left">&nbsp;</td>\
            </tr>\
            <tr>\
              <td height="106" colspan="2" align="center" class="style5">\
                <table width="95%" border="0" cellspacing="0" cellpadding="0">\
                  <tr>\
                    <td><img src="images/icon05.png" width="67" height="67" /></td>\
                    <td>&nbsp;</td>\
                    <td align="left"><span>' + RpWlanNoWireless + '</span></td>\
                  </tr>\
                </table>\
              </td>\
            </tr>\
            <tr>\
              <td colspan="2" align="center">\
                <table border="0" cellspacing="0" cellpadding="0">\
                  <tr>\
                    <td><table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                      <tr>\
                        <td class="button1" onclick="goToWirelessAdvancedPage()"><a>' + SelectTypeWarnOK + '</a></td>\
                      </tr>\
                    </table></td>\
                    <td>&nbsp;</td>\
                    <td><table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                      <tr>\
                        <td class="button1" onclick="skipEnableWireless()"><a>' + DashCancel + '</a></td>\
                      </tr>\
                    </table></td>\
                  </tr>\
                </table>\
              </td>\
            </tr>\
          </table></td>\
        </tr>\
      </table></td>\
    </tr>\
  </table>');
}
