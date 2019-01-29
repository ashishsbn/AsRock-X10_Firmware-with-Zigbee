/* wizard.js -- specific javascript for ASRock X10 EZ setup wizard */
function writeNoWanWindow()
{
  document.write('\
  <div id="noWanWindow" style="display:none"> \
    <table width="400" height="180" border="0" cellspacing="0" cellpadding="0" bgcolor="#333333">\
      <tr>\
        <td valign=top>\
          <table width="100%" border="0" cellspacing="0" cellpadding="0">\
            <tr>\
              <td width="92%" align="left">&nbsp;</td>\
            </tr>\
            <tr>\
              <td height="106" colspan="2" align="center" class="style5" id="rebootModem" style="display:none">\
                <table width="95%" border="0" cellspacing="0" cellpadding="0">\
                  <tr>\
                    <td><img src="images/icon05.png" width="67" height="67" /></td>\
                    <td>&nbsp;</td>\
                    <td align="left"><span>' + SelectTypeWarnDHCP + '</span></td>\
                  </tr>\
                </table>\
              </td>\
              <td height="106" colspan="2" align="center" class="style5" id="callIsp" style="display:none">\
                <table width="95%" border="0" cellspacing="0" cellpadding="0">\
                  <tr>\
                    <td><img src="images/icon05.png" width="67" height="67" /></td>\
                    <td>&nbsp;</td>\
                    <td align="left"><span>' + PopInfo18 + '</span></td>\
                  </tr>\
                </table>\
              </td>\
            </tr>\
            <tr>\
              <td colspan="2" align="center"><table border="0" cellspacing="0" cellpadding="0">\
                <tr>\
                  <td><table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                    <tr>\
                      <td class="button1" id="reSelectWan"><a>' + SelectTypeWarnOK + '</a></td>\
                    </tr>\
                  </table></td>\
                </tr>\
              </table></td>\
            </tr>\
          </table></td>\
        </tr>\
      </table></td>\
    </tr>\
  </table>');
}

// 120 * 3 = 360 seconds = 6 minutes
var countdownUpgradeFwTotal = countdownUpgradeFwRemaining = 120;
var countdownOnUpgrade = 0;
var retryUpgradeFw = 0, fwDownloading = 0, fwUpgrading = 0;

function checkFwDownload()
{
  $.ajax({
    url: "/f/wizardGetFwDownload",
    type: "GET",
    success: function(result) {
      if (parseInt(result.result) == -1) {
        if (retryUpgradeFw == 0) {
          alert(DownloadWarn2);
          retryUpgradeFw = 1;
          fwUpgrade();
        }
        else {
          alert(DownloadWarn3);
          $('#upgradeFwCountdown').hide();
          goToWirelessSettings();
        }
      }
      else {
        if (parseInt(result.result) == 1) {
          fwDownloading = 0;
          sendFwUpgrade();
        }
        setTimeout(countdownUpgradeFwProcess, 3000);
      }
    },
    error : function() {
      console.log("get /f/wizardGetFwDownload error");
      setTimeout(countdownUpgradeFwProcess, 3000);
    }
  });
}

function checkFwUpgrade()
{
  //Check after 80*3=240 seconds
  if ((countdownOnUpgrade - countdownUpgradeFwRemaining) >= 80) {
    $.ajax({
      url: "/f/wizardGetDefault",
      type: "GET",
      dataType: "jsonp",
      crossDomain: false,
      success: function(result) {
        $('#upgradeFwCountdown').hide();
        goToWirelessSettings();
      },
      error : function() {
        console.log("get /f/wizardGetDefault error");
        setTimeout(countdownUpgradeFwProcess, 3000);
      }
    });
  }
  else {
    setTimeout(countdownUpgradeFwProcess, 3000);
  }
}

function sendFwUpgrade()
{
  $.ajax({
    url: "/f/wizardUpgradeFw",
    type: "POST",
    dataType: "json",
    success: function(result) {
      fwUpgrading = 1;
      countdownOnUpgrade = countdownUpgradeFwRemaining;
    },
    error : function() {
      console.log("get /f/wizardUpgradeFw error");
    }
  });
}

function countdownUpgradeFwProcess()
{
  var szazalek=Math.ceil(((countdownUpgradeFwTotal-countdownUpgradeFwRemaining)/countdownUpgradeFwTotal)*100);
  var born=3.4*szazalek;
  
  document.getElementById("szliderbar2").style.width=born+'px';
  document.getElementById("precentNumber2").innerHTML=szazalek+'%';
  
  if (countdownUpgradeFwRemaining <= 0) {
    $('#upgradeFwCountdown').hide();
    goToWirelessSettings();
  } 
  else {
    countdownUpgradeFwRemaining--;
    if (fwDownloading == 1) {
      checkFwDownload();
    }
    else {
      checkFwUpgrade();
    }
  }
}

function fwUpgrade()
{
  countdownUpgradeFwTotal = countdownUpgradeFwRemaining = 160;
  fwDownloading = fwUpgrading = countdownOnUpgrade = 0;
  $.ajax({
    url: "/f/wizardDownloadFw",
    type: "POST",
    dataType: "json",
    success: function(result) {
      console.log("get /f/wizardUpgradeFw success");
    },
    error : function() {
      console.log("get /f/wizardUpgradeFw error");
    }
  });
  $('#haveNewFw').hide();
  $('#upgradeFwCountdown').show();
  fwDownloading = 1;
  countdownUpgradeFwProcess();
}
