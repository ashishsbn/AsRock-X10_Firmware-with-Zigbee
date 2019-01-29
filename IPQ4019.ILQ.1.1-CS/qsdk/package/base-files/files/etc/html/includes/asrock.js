/* asrock.js -- specific javascript for ASRock Router */
function MM_swapImgRestore() { //v3.0
  var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}
function MM_preloadImages() { //v3.0
  var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
    var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
    if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}
}

function MM_findObj(n, d) { //v4.01
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && d.getElementById) x=d.getElementById(n); return x;
}

function MM_swapImage() { //v3.0
  var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}
}

function preloadX10Images() {
  MM_preloadImages('images/UI-new-over_03.jpg', 'images/UI-new-over_04.jpg', 'images/UI-new-over_05.jpg', 'images/UI-new-over_06.jpg', 'images/botton06-on.png', 'images/botton06-s-on_01.png', 'images/icon05.png', 'images/bar-bg.jpg', 'images/bar.jpg');
}

function msIeVersion() {
  var sAgent = window.navigator.userAgent;
  var Idx = sAgent.indexOf("MSIE");
 
  // If IE, return version number.
  if (Idx > 0)
    return parseInt(sAgent.substring(Idx+ 5, sAgent.indexOf(".", Idx)));
  // If IE 11 then look for Updated user agent string.
  else if (!!navigator.userAgent.match(/Trident\/7\./))
    return 11;
  else
    return 0; //It is not IE
}

function getOpMode()
{
  var currentPage;

  if (newOpMode != -1) {
    return newOpMode;
  }

  currentPage = window.location.toString();
  if (currentPage.indexOf("/ap/") > -1) {
    return 2;
  }
  else if (currentPage.indexOf("/br/") > -1) {
    return 3;
  }
  else if (currentPage.indexOf("/rp/") > -1) {
    return 1;
  }
  else {
    return 0;
  }
}

function showWindow(id)
{
  $('#cover').show();
  $('#' + id).show();
}

function closeWindow(id)
{
  $('#cover').hide();
  $('#' + id).hide();
}

var rebootTimeoutOnRepeaterMode = 180;
var totalTime, remainingTime, waitTimeout = 15, rebootTimeout = 85;
var hostipaddr, hosturl;
var requiredData = {}, online = 0;
var checkHost = 0, newHost = "";
var newOpMode = -1;

function setOpModePara(newMode)
{
  newOpMode = newMode;
  checkHost = 1;
  newHost = "asrock.router";
}

function checkOnline(refreshPage)
{
  var url = "", alivePage, cross = false;
  var opMode = getOpMode();
  
  switch(opMode) {
    case 1:
      alivePage = "/f/rp/wizardGetDefault"
      break;
    case 2:
      alivePage = "/f/ap/wizardGetDefault"
      break;
    case 3:
      alivePage = "/f/br/wizardGetDefault"
      break;
    case 0:
    default:
      alivePage = "/f/wizardGetDefault"
      break;
  }
  
  if (checkHost == 1 && newHost != "" && newHost != window.location.host) {
    url = "http://" + newHost + alivePage;
    cross = true;
  }
  else {
    url = alivePage;
  }
  
  requiredData.online = 0;
  $.ajax({
    url: url,
    type: "GET",
    dataType: "jsonp",
    crossDomain: cross,
    success: function(result) {
      console.log("get " + url + " success");
      requiredData.online = 1;
      if (refreshPage == 1) {
        var currentPage = window.location.toString();
        if (currentPage.indexOf("wizard") != -1) {
          goToHomePage();
        }
        else {
          refresh();
        }
      }
    },
    error : function() {
      console.log("get " + url + " error");
    }
  });
}

function countdown()
{
  var szazalek=Math.ceil(((totalTime-remainingTime)/totalTime)*100);
  var born=3.4*szazalek;
  var currentPage;
  
  document.getElementById("szliderbar").style.width=born+'px';
  document.getElementById("precentNumber").innerHTML=szazalek+'%';
  
  if (remainingTime <= 0) {
    if (parseInt(requiredData.online) == 1) {
      currentPage = window.location.toString();
      if (currentPage.indexOf("adv_wlan_rescan.html") != -1 || currentPage.indexOf("wizard") != -1) {
        goToHomePage();
      }
      else {
        refresh();
      }
    }
    else {
      var tmpOpMode;
      if (newOpMode != -1) {
        tmpOpMode = newOpMode;
      }
      else {
        tmpOpMode = getOpMode();
      }
      switch(tmpOpMode) {
        case 0:
        case 1:
          $('span#disconnectedSsid2g').html(requiredData.ssid2g);
          $('span#disconnectedPsk2g').html(requiredData.psk2g);
          $('span#disconnectedSsid5g').html(requiredData.ssid5g);
          $('span#disconnectedPsk5g').html(requiredData.psk5g);
          $('#disconnectedDig').show();
          break;
        case 2:
          $('#noDhcpIpDig').show();
          break;
        case 3:
          $('#brDisconnectedDig').show();
          break;
	  }

      $('#waitingDig').hide();
      $('#rebootingDig').hide();
      $('#fwUpgradeDig').hide();
      $('#progressBar').hide();
      $('#afterRebootDig').show();
    }
  } 
  else {
    if (remainingTime == 3) {
      checkOnline(0);
    }

    remainingTime--;
    setTimeout(countdown, 1000);
  }
}

function countdownWindow(id)
{
  if (id == "waitingDig" || id == "fwUpgradeDig") {
    checkHost = 0;
    totalTime = remainingTime = waitTimeout;
  }
  else if (id == "rebootingDig") {
    checkHost = 1;
    totalTime = remainingTime = rebootTimeout;
  }
  
  $('#' + id).show();
  $('#progressBar').show();
  
  countdown();
}

function writeWaitWindow()
{
  document.write('\
  <div id="waitWindow" style="display:none"> \
    <table width="400" height="180" border="0" cellspacing="0" cellpadding="0" bgcolor="#333333"> \
      <tr> \
        <td valign="top"> \
          <table width="100%" border="0" cellspacing="0" cellpadding="0"> \
            <tr> \
              <td width="92%" align="left">&nbsp;</td> \
            </tr> \
            <tr> \
              <td height="106" colspan="2" align="center" class="style5" id="waitingDig" style="display:none"> \
                <table width="95%" height="100%" border="0" cellspacing="0" cellpadding="0"> \
                  <tr> \
                    <td><img src="images/icon05.png" width="67" height="67" /></td> \
                    <td>&nbsp;</td> \
                    <td align="left"><span>' + HomeInfo + '</span></td> \
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr> \
              <td height="106" colspan="2" align="center" class="style5" id="fwUpgradeDig" style="display:none"> \
                <table width="95%" height="100%" border="0" cellspacing="0" cellpadding="0"> \
                  <tr> \
                    <td><img src="images/icon05.png" width="67" height="67" /></td> \
                    <td>&nbsp;</td> \
                    <td align="left"><span>' + StartCheckFWUpdate + '</span></td> \
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr> \
              <td height="106" colspan="2" align="center" class="style5" id="confirmApplyDig" style="display:none"> \
                <table width="95%" height="100%" border="0" cellspacing="0" cellpadding="0"> \
                  <tr> \
                    <td><img src="images/icon05.png" width="67" height="67" /></td> \
                    <td>&nbsp;</td> \
                    <td align="left"><span>' + DashOnOffConfirm + '</span></td> \
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr> \
              <td height="106" colspan="2" align="center" class="style5" id="confirmUpgradeDig" style="display:none"> \
                <table width="95%" height="100%" border="0" cellspacing="0" cellpadding="0"> \
                  <tr> \
                    <td><img src="images/icon05.png" width="67" height="67" /></td> \
                    <td>&nbsp;</td> \
                    <td align="left"><span>' + DashUpgradeFwConfirm + '</span></td> \
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr> \
              <td height="106" colspan="2" align="center" class="style5" id="rebootingDig" style="display:none"> \
                <table width="95%" height="100%" border="0" cellspacing="0" cellpadding="0"> \
                  <tr> \
                    <td><img src="images/icon05.png" width="67" height="67" /></td> \
                    <td>&nbsp;</td> \
                    <td align="left"><span>' + SetupWarn7 + '</span></td> \
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr> \
              <td height="106" colspan="2" align="center" class="style5" id="disconnectedDig" style="display:none"> \
                <table width="95%" height="100%" border="0" cellspacing="0" cellpadding="0"> \
                  <tr> \
                    <td><img src="images/icon05.png" width="67" height="67" valign="middle" /></td> \
                    <td>&nbsp;</td> \
                    <td align="left"><h4>' + SetupWarn2 + '</h4> \
                      <span style="line-height: 1.5;">' + SetupWarn3 + '</span> \
                        <p><b>' + Dash24G + '&nbsp;' + DashGuestSSID + ':</b>&nbsp;<span id="disconnectedSsid2g"></span><br> \
                        ' + SetupWarnKey + ':&nbsp;<span id="disconnectedPsk2g"></span><br> \
                        <b>' + Dash5G + '&nbsp;' + DashGuestSSID + ':</b>&nbsp;<span id="disconnectedSsid5g"></span><br> \
                        ' + SetupWarnKey + ':</b>&nbsp;<span id="disconnectedPsk5g"></span></p> \
                      <span style="line-height: 1.5;">' + SetupWarn4 + '</span> \
                    </td> \
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr> \
              <td height="106" colspan="2" align="center" class="style5" id="brDisconnectedDig" style="display:none"> \
                <table width="95%" height="100%" border="0" cellspacing="0" cellpadding="0"> \
                  <tr> \
                    <td><img src="images/icon05.png" width="67" height="67" valign="middle" /></td> \
                    <td>&nbsp;</td> \
                    <td align="left"><h4>' + SetupWarn2 + '</h4> \
                      <span style="line-height: 1.5;">' + SetupWarn4 + '</span> \
                    </td> \
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr> \
              <td height="106" colspan="2" align="center" class="style5" id="noDhcpIpDig" style="display:none"> \
                <table width="95%" height="100%" border="0" cellspacing="0" cellpadding="0"> \
                  <tr> \
                    <td><img src="images/icon05.png" width="67" height="67" /></td> \
                    <td>&nbsp;</td> \
                    <td align="left"><span>' + ApLanIpWarn + '</span></td> \
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr> \
              <td align="center" colspan="2" id="progressBar" style="display:none"> \
                <table width="100%" cellspacing="0" cellpadding="0" border="0"> \
                  <tr> \
                    <td width="8%"></td> \
                    <td width="92%"> \
                      <div align="left" id="szlider"> \
                        <div id="precentNumber"></div> \
                        <div id="szazalek"></div> \
                        <div id="szliderbar"></div> \
                      </div> \
                    </td> \
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr><td>&nbsp;</td></tr> \
            <tr> \
              <td colspan="2" align="center"> \
                <table width="119" height="39" border="0" cellspacing="0" cellpadding="0" id="applyConfirm" style="display:none"> \
                  <tr> \
                    <td>\
                      <table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                        <tr>\
                          <td class="button1" id="confirmToApply"><a>' + DashApply + '</a></td>\
                        </tr>\
                      </table>\
                    </td>\
                    <td>&nbsp;</td>\
                    <td>\
                      <table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                        <tr>\
                          <td class="button1" id="confirmToCancel"><a>' + DashCancel + '</a></td>\
                        </tr>\
                      </table>\
                    </td>\
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr> \
              <td colspan="2" align="center"> \
                <table width="119" height="39" border="0" cellspacing="0" cellpadding="0" id="afterRebootDig" style="display:none"> \
                  <tr> \
                    <td class="button1" id="reCheckOnline"><a>' + SelectTypeWarnOK + '</a></td> \
                  </tr> \
                </table> \
              </td> \
            </tr> \
            <tr><td>&nbsp;</td></tr> \
          </table> \
        </td> \
      </tr> \
    </table> \
  </div>');
}

function padZero(str, max, left)
{
  var i, src, dest;
  
  src = str.toString();
  
  if (left) {
    dest = "";
    for (i = 0; i<(max-src.length); i++) {
      dest += "0";
    }
    dest += src;
  }
  else {
    dest = src;
    for (i = 0; i<(max-src.length); i++) {
      dest += "0";
    }
  }
  
  return dest;
}

function findElementByID(id)
{
  element=document.getElementById(id);
  return element;
}

function changeTimeOnOffStat(thisSwitch)
{
  var imageOff = document.createElement("img");
  imageOff.src = "images/icon02-off.png";
  imageOff.id = "imageOff";
  imageOff.style.width = "18px";
  imageOff.style.height = "18px";
  var imageOn = document.createElement("img");
  imageOn.src = "images/icon02-on.png";
  imageOn.id = "imageOn";
  imageOn.style.width = "18px";
  imageOn.style.height = "18px";
  var c = thisSwitch.children;
  if(c[0].id=="imageOn"){
    thisSwitch.replaceChild(imageOff,c[0]);
  }
  else{
    thisSwitch.replaceChild(imageOn,c[0]);
  }  
}
var weekName = new Array("0","schMon", "schTue", "schWed", "schThu", "schFri", "schSat", "schSun");
function showdiv(id)
{
  var cover = document.getElementById("cover");
  cover.style.display="block";
  var dlg=document.getElementById(id);
    $('#' + id).draggable();
    dlg.style.position = "fixed";
    dlg.style.zIndex = 910;
    dlg.style.top = "10%";
    dlg.style.left = "36%";
    $('#' + id).show();
}

function setSCHData(name, url, switchThis)
{
  var argu, ssid, broadcast, rts, dtim;
  var authMode, radiusIp, radiusPort, radiusKey;

  if (name == "kid5Gonoff") {
    argu = name;
    argu += "&enable=1";
    argu += "&schMon=" + $("#schMon5G").val();
    argu += "&schTue=" + $("#schTue5G").val();
    argu += "&schThu=" + $("#schThu5G").val();
    argu += "&schWed=" + $("#schWed5G").val();
    argu += "&schFri=" + $("#schFri5G").val();
    argu += "&schSat=" + $("#schSat5G").val();
    argu += "&schSun=" + $("#schSun5G").val();
  }
  else {
    argu = name;
    argu += "&enable=1";
    argu += "&schMon=" + $("#schMon").val();
    argu += "&schTue=" + $("#schTue").val();
    argu += "&schThu=" + $("#schThu").val();
    argu += "&schWed=" + $("#schWed").val();
    argu += "&schFri=" + $("#schFri").val();
    argu += "&schSat=" + $("#schSat").val();
    argu += "&schSun=" + $("#schSun").val();
  }
  console.log(argu);

  $.ajax({
    url: url,
    type: "POST",
    data: argu,
    dataType: "json",
    success: function(result) {
      console.log(result);
    },
    error : function() {
      console.log("post " + url + " error");
    }
  });

  return true;
}

function gotoSetSCH(name)
{  
  console.log("set enable param schedule => %s\n",parseValueOfOnOff($('#enableSchedule').val()));

	if(name == "kid5G"){
		console.log("kid 5G schedule setting");
		  for(week = 1; week <= 7; week++){
			timer="";
			for(i = 0 ; i < 24 ; i++){
			  if(document.getElementById("sch5G"+week+i).children[0].id=="imageOn")
				timer=timer+"1";
			  else
				timer=timer+"0";
			}
			$("#"+weekName[week]+"5G").val(timer)
		  console.log("set to kid5Gschedule => %s\n",$("#"+weekName[week]+"5G").val());
		 
		 }
	}else{
		  for(week = 1; week <= 7; week++){
			timer="";
			for(i = 0 ; i < 24 ; i++){
			  if(document.getElementById("sch"+week+i).children[0].id=="imageOn")
				timer=timer+"1";
			  else
				timer=timer+"0";
			}
			$("#"+weekName[week]).val(timer)
		  console.log("set to schedule => %s\n",$("#"+weekName[week]).val());
		 
		 }
	}
}
function getSCHData(url,name,switchThis)
{
  var schInfo = getData(url,name);
  if (schInfo != null) { 
     /* Put the value into the DOM by key (id) */
    $.each(schInfo, function(key, value) {
      console.log("key : " + key + " value : " + value);
	  if(name == "kid5Gonoff"){
		  if(key == "schMon" ||key == "schThu"||key == "schWed"||key == "schTue"||key == "schFri"||key == "schSat"||key == "schSun")
			  $("#" + key+"5G").val(value);
	  }else{
        $("#" + key).val(value);
	  }
     if(key == "enable" && switchThis !== undefined && switchThis !== null){                                                  
       var schedule_enable = parseInt(value);        
       if(schedule_enable)  
         $("#"+switchThis).val("ON");
       else                                                   
         $("#"+switchThis).val("OFF");
     }                                                        
	});
  }
  else {
    console.log("schedule data is null");
  }
  if(name == "selfhealing"){                                                                             
	  for( week = 1 ; week <=7 ; week++){                                    
	   var sch=$("#"+weekName[week]).val();                                  
	   for(i=0;i<24;i++){                                                       
		 if(sch.substring(i,i+1)=="1")                                          
		   changeTimeOnOffStat(findElementByID("sch"+week+i));               
		}                                                                     
	  }
	  $("#schName").val(name);  
  }else if(name == "kid5Gonoff"){                                                                             
	  for( week = 1 ; week <=7 ; week++){                                    
	   var sch=$("#"+weekName[week]+"5G").val();                                  
	   for(i=0;i<24;i++){                                                       
		 if(sch.substring(i,i+1)=="0")                                          
		   changeTimeOnOffStat(findElementByID("sch5G"+week+i));               
		}                                                                     
	  }
	  $("#schName5G").val(name);  
	  console.log($("#schName5G").val(name));
  }else{
	  for( week = 1 ; week <=7 ; week++){                                    
	   var sch=$("#"+weekName[week]).val();                                  
	   for(i=0;i<24;i++){                                                       
		 if(sch.substring(i,i+1)=="0")                                          
		   changeTimeOnOffStat(findElementByID("sch"+week+i));               
		}                                                                     
	  }
	    $("#schName").val(name);  
  }
}
function ClearSCHData()
{
	  for( week = 1 ; week <=7 ; week++){                                    
	   $("#"+weekName[week]).val("111111111111111111111111");                                  
	   for(i=0;i<24;i++){                                                       
		 findElementByID("sch"+week+i).value="on";               
		}                                                                     
	  }
}
function writeScheduleWindow()
{
  var firstRow=[ParTime, ParMon, ParTue, ParWed, ParThu, ParFri, ParSat, ParSun];
  
  //Start of Header
  document.write('\
    <div id="scheduleWindow" style="display:none">\
      <table width="502" border="0" cellspacing="0" cellpadding="0">\
        <tbody>');
  //Title
  document.write('\
          <tr>\
            <td background="images/box_01.png" width="502" height="46">\
              <table width="100%" border="0" cellspacing="0" cellpadding="0">\
                <tbody>\
                  <tr>\
                    <td width="92%" align="left"><p>' + ParTimeManage + '</p></td>\
                    <td width="8%" align="left">\
                      <a href="#" onmouseout="MM_swapImage(\'Image24\',\'\',\'images/botton05-nol.png\',1)" onmouseover="MM_swapImage(\'Image24\',\'\',\'images/botton05-over.png\',1)"><img src="images/botton05-nol.png" name="Image24" width="26" height="26" border="0" id="Image24" onclick="closeWindow(\'scheduleWindow\')"></a>\
                    </td>\
                  </tr>\
                </tbody>\
              </table>\
            </td>\
          </tr>');
  //Start of Table
  document.write('\
          <tr>\
            <td width="502" align="center" background="images/box_03.png">\
              <table width="96%" border="0" cellpadding="3" cellspacing="1">\
                <tbody>');
  //First row
  document.write('<tr>');
  for (i=0; i<8; i++) {
    document.write('<td width="12%" align="center" bgcolor="#666666"><h5>' + firstRow[i] + '</h5></td>');
  }
  document.write('</tr>');
  //Each Hour
  for (i=0; i<24; i++) {
    var bgColor=" ";
    if (i%2 > 0) {
      bgColor=" bgcolor=\"#494949\" ";
    }
    document.write('<tr>');
    document.write('<td align="center"' + bgColor +'class="style3">' + padZero(i, 2, 1) + '~' + padZero(i+1, 2, 1) + '</td>');
    for (j=0; j<7; j++) {
      document.write('<td align="center"' + bgColor +'class="style5">\
                         <span id=sch' + (j+1) + '' + i + 'value="ON" onclick="changeTimeOnOffStat(this);">\
                           <img id="imageOn" src="images/icon02-on.png" width="18" height="18">\
                         </span>\
                      </td>');
    }
    document.write('</tr>');
  }
  //End of Table
  document.write('\
                </tbody>\
              </table>\
            </td>\
          </tr>');
  //Buttons
  document.write('\
          <tr>\
            <td align="center" background="images/box_03.png">\
              <table border="0" cellspacing="0" cellpadding="0">\
                <tbody>\
                  <tr>\
                    <td>\
                      <table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                        <tr>\
                          <td class="button1" id="Apply"><a>' + DashApply + '</a></td> \
                        </tr>\
                      </table>\
                    </td>\
                    <td>&nbsp;</td>\
                    <td>\
                      <table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                        <tr>\
                          <td class="button1" id="Cancel"><a>' + DashCancel + '</a></td> \
                        </tr>\
                      </table>\
                    </td>\
                  </tr>\
                </tbody>\
              </table>\
            </td>\
          </tr>');
  //Last Row
  document.write('<tr><td><img src="images/box_04.png" width="502" height="28"></td></tr>');
  //End of Header
  document.write('\
        </tbody>\
      </table>\
    </div>');
}

function writeIpWindow(msg)
{
  document.write('\
    <div id="ipWindow" style="display:none">\
      <table width="400" height="180" border="0" cellspacing="0" cellpadding="0" bgcolor="#333333">\
        <tr>\
          <td valign="top">\
            <table width="100%" border="0" cellspacing="0" cellpadding="0">\
              <tr>\
                <td width="92%" align="center">&nbsp;</td>\
              </tr>\
              <tr>\
                <td width="92%" align="center">&nbsp;</td>\
              </tr>\
              <tr>\
                <td align="center" class="style5">' + msg + '</td>\
              </tr>\
              <tr>\
                <td width="92%" align="center">&nbsp;</td>\
              </tr>\
              <tr>\
                <td align="center">\
                  <input type="text" id="ipWindowIpAddr" value="" size="25" list="clientInfo">\
                  <datalist id="ipClientInfo"></datalist>\
                </td>\
              </tr>\
              <tr>\
                <td width="92%" align="center">&nbsp;</td>\
              </tr>');
  // Buttons
  document.write('\
              <tr>\
                <td align="center">\
                  <table border="0" cellspacing="0" cellpadding="0">\
                    <tbody>\
                      <tr>\
                        <td>\
                          <table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                            <tr>\
                              <td class="button1" id="ipWindowApply"><a>' + DashApply + '</a></td>\
                            </tr>\
                          </table>\
                        </td>\
                        <td>&nbsp;</td>\
                        <td>\
                          <table width="119" height="39" border="0" cellspacing="0" cellpadding="0">\
                            <tr>\
                              <td class="button1" id="ipWindowCancel"><a>' + DashCancel + '</a></td>\
                            </tr>\
                          </table>\
                        </td>\
                      </tr>\
                    </tbody>\
                  </table>\
                </td>\
              </tr>');
  document.write('\
              <tr><td>&nbsp;</td></tr> \
            </table> \
          </td> \
        </tr>\
      </table>\
    </div>');
}

function writeCover()
{
  document.write('<div id="cover" style="display:none"></div>');
}

function writeWindow(id)
{
  if (id == "cover") {
    writeCover();
  }
  else if (id == "waitWindow") {
    writeWaitWindow();
  }
  else if (id=="scheduleWindow") {
    writeScheduleWindow();
  }
  else if (id=="ipWindow") {
    writeIpWindow(WANPortForwardingEnterIP);
  }
  else if (id=="dashboardWanWindow") {
    writeDashboardWanWindow(id);
  }
  else if (id=="dashboardRouterWindow") {
    writeDashboardRouterWindow(id);
  }
  else if (id=="dashboardClientWindow") {
    writeDashboardClientWindow(id);
  }
  else if (id=="dashboardUsbWindow") {
    writeDashboardUsbWindow(id);
  }
  else if (id=="noWanWindow") {
    writeNoWanWindow();
  }
  else if (id=="noWirelessWindow") {
    writeNoWirelessWindow();
  }
  else if (id=="dashboardApLanWindow") {
    writeDashboardApLanWindow(id);
  }
  else if (id=="dashboardApWindow") {
    writeDashboardApWindow(id);
  }
  else if (id=="dashboardRpLanWindow") {
    writeDashboardRpLanWindow(id);
  }
  else if (id=="dashboardRpWindow") {
    writeDashboardRpWindow(id);
  }
  else if (id=="dashboardBrLanWindow") {
    writeDashboardBrLanWindow(id);
  }
  else if (id=="dashboardBrWindow") {
    writeDashboardBrWindow(id);
  }
  
  setWindow(id);
}

function setWindow(id)
{
  var dlg = document.getElementById(id);
  if (id == "cover") {
    dlg.style.position = "fixed";
    dlg.style.top = "0px";
    dlg.style.left = "0px";
    dlg.style.opacity = "0.75";
    dlg.style.filter = 'progid:DXImageTransform.Microsoft.Alpha(opacity=75)';
    dlg.style.backgroundColor = "black";
    dlg.style.width = "100%";
    dlg.style.height = "100%";
    dlg.style.zIndex = 900;
  }
  else if (id == "waitWindow") {
    dlg.style.position = "fixed";
    dlg.style.zIndex = 910;
    dlg.style.top = "50%";
    dlg.style.left = "50%";
    dlg.style.marginLeft = "-200px";
    dlg.style.marginTop = "-100px";
  }
  else if (id == "scheduleWindow") {
    $('#' + id).draggable();
    dlg.style.position = "fixed";
    dlg.style.zIndex = 910;
    dlg.style.top = "10%";
    dlg.style.left = "36%";
  }
  else if (id == "ipWindow") {
    dlg.style.position = "fixed";
    dlg.style.zIndex = 910;
    dlg.style.top = "25%";
    dlg.style.left = "36%";
  }
  else if (id.indexOf("dashboard") > -1) {
    dlg.style.position = "fixed";
    dlg.style.zIndex = 910;
    dlg.style.top = "50%";
    dlg.style.left = "50%";
    var width = $("#" + id).width();
    dlg.style.marginLeft = "-" + width/2 + "px";
    var height = $("#" + id).height();
    dlg.style.marginTop = "-" + height/2 + "px";
  }
  else if (id == "noWanWindow" || id == "noWirelessWindow") {
    dlg.style.position = "fixed";
    dlg.style.zIndex = 910;
    dlg.style.top = "50%";
    dlg.style.left = "50%";
    dlg.style.marginLeft = "-200px"
    dlg.style.marginTop = "-150px"
  }
}
function refresh(page)
{
  if (checkHost == 1 && newHost != "" && newHost != window.location.host) {
    if (page) {
      page = "http://" + newHost + "/" + page;
    }
    else {
      page = "http://" + newHost + window.location.pathname;
    }
    window.open(page, "_self");
  }
  else {
    if (page) {
      page = "http://"+window.location.host+"/"+page;
      window.open(page, "_self");
    }
    else {
      location.reload(true);
    }
  }
}

if(typeof console === "undefined") { var console = { log: function (logMsg) { } }; }

var switchArray=[], textArray=[], selectArray=[], fontArray=[];
function emptyElementArrays()
{
  switchArray=[];
  textArray=[];
  selectArray=[];
  fontArray=[];
}

function isInArray(value, array)
{
  return $.inArray(value, array) > -1;
}

function doAssignValues(key, value)
{
  if (isInArray(key, switchArray)) {
    if (value == "0") {
      value = "OFF";
    }
    else if (value == "1") {
      value = "ON";
    }
    $('#' + key).val(value);
    if (value == "ON") {
      $('span#' + key).trigger("click");
    }
  }
  else if (isInArray(key, textArray)) {
    $('input#' + key).val(value);
  }
  else if (isInArray(key, selectArray)) {
    $('select#' + key).val(value);
  }
  else if (isInArray(key, fontArray)) {
    $('font#' + key).html(value);
  }
}

function getData(url, argu)
{
  var jsonData = {};
  
  $.ajaxSetup({
    async: false
  });
  
  $.ajax({
    url: url,
    type: "GET",
    dataType: "json",
    data: argu,
    success: function(result) {
      jsonData = result;
    },
    error : function() {
      console.log("get " + url + " error");
      jsonData = null;
    }
  });
  
  $.ajaxSetup({
    async: true
  });

  return jsonData;
}

function parseValueOfOnOff(value)
{
  if (value == "ON") {
    return "1";
  }
  else {
    return "0"
  }
}

function label(label, id)
{
  $('#' + id).append("<td align=\"left\" class=\"style6\" height=\"30\" width=\"35%\">" + label + "</td>");
}

function text(label, id, sepText, mask)
{
  $('#' + id).append("<td align=\"left\" class=\"style6\" height=\"30\" width=\"35%\">" + label + "</td>");
  if (sepText !== undefined && sepText !== null) {
    if (mask !== undefined && mask == 1) {
      $('#' + id).append("<td align=\"left\" class=\"style6-2\" width=\"65%\"><input name=\"" 
                        + id + "\" id=\"" + id + "\" type=\"password\"></input>&nbsp;" + sepText + "</td>");
    }
    else {
      $('#' + id).append("<td align=\"left\" class=\"style6-2\" width=\"65%\"><input name=\"" 
                        + id + "\" id=\"" + id + "\"></input>&nbsp;" + sepText + "</td>");
    }
  }
  else {
    if (mask !== undefined && mask == 1) {
      $('#' + id).append("<td align=\"left\" class=\"style6-2\" width=\"65%\"><input name=\"" 
                        + id + "\" id=\"" + id + "\" type=\"password\"></input></td>");
    }
    else {
      $('#' + id).append("<td align=\"left\" class=\"style6-2\" width=\"65%\"><input name=\"" 
                        + id + "\" id=\"" + id + "\"></input></td>");
    }
  }
  
  textArray.push(id);
}

function button(label, id, value, callback)
{
  $('#' + id).append("<td align=\"left\" class=\"style6\" height=\"30\" >" + label + "</td>");
  $('#' + id).append("<td align=\"left\" class=\"style6-2\"><input type=\"button\" id=\"" 
		      + id + "\" value=\"" + value + "\"</input></td>");

  $('input#' + id).click(function() {
    if (callback !== undefined) callback();
  });
}

function switchOnOff(label, id, callback, width)
{
  var localWidth;
  if(width !== undefined) {
    localWidth = width;
  }
  else {
    localWidth = 35;
  }
  console.log("%s\n",$('#' + id).val());
  if($('#' + id).val()=="ON"){
	  $('#' + id).append("<td align=\"left\" width=\"" + localWidth + "%\" valign=\"middle\" class=\"style6\" >" + label + "</td>");
	  $('#' + id).append("<td align=\"left\" width=\"" + (100-localWidth) + "%\"><span id=\"" + id + "\" class=\"" 
				  + id + "\"><img id=\"imageOff\" src=\"images/botton03-on.png\" width=\"68\" height=\"30\"></span></td>");
	  $('#' + id).val("ON");
  }else{
	  $('#' + id).append("<td align=\"left\" width=\"" + localWidth + "%\" valign=\"middle\" class=\"style6\" >" + label + "</td>");
	  $('#' + id).append("<td align=\"left\" width=\"" + (100-localWidth) + "%\"><span id=\"" + id + "\" class=\"" 
				  + id + "\"><img id=\"imageOff\" src=\"images/botton03-off.png\" width=\"68\" height=\"30\"></span></td>");
	  $('#' + id).val("OFF");
  }
  $('span#' + id).click(function() {
    changeOnOffStat(this);
    if (callback !== undefined && callback !== null) callback(this);
  });
  
  switchArray.push(id);
}

function switchOnOffDisable(label, id, width)
{
  var localWidth;
  if(width !== undefined) {
    localWidth = width;
  }
  else {
    localWidth = 35;
  }
  console.log("%s\n",$('#' + id).val());
  if($('#' + id).val()=="ON"){
    $('#' + id).append("<td align=\"left\" width=\"" + localWidth + "%\" valign=\"middle\" class=\"style6\" >" + label + "</td>");
    $('#' + id).append("<td align=\"left\" width=\"" + (100-localWidth) + "%\"><span id=\"" + id + "\" class=\"" 
          + id + "\"><img id=\"imageOff\" src=\"images/botton03-on.png\" width=\"68\" height=\"30\"></span></td>");
    $('#' + id).val("ON");
  }else{
    $('#' + id).append("<td align=\"left\" width=\"" + localWidth + "%\" valign=\"middle\" class=\"style6\" >" + label + "</td>");
    $('#' + id).append("<td align=\"left\" width=\"" + (100-localWidth) + "%\"><span id=\"" + id + "\" class=\"" 
          + id + "\"><img id=\"imageOff\" src=\"images/botton03-off.png\" width=\"68\" height=\"30\"></span></td>");
    $('#' + id).val("OFF");
  }
  
  switchArray.push(id);
}

function select(label, id, lists, callback)
{
  $('#' + id).append("<td align=\"left\" class=\"style6\" height=\"30\" width=\"35%\">" + label + "</td>");
  $('#' + id).append("<td align=\"left\" width=\"65%\"><select size=\"1\" name=\"" + id + "\" id=\"" + id + "\"></select></td>");
  
  $.each(lists, function(index, value) {
    $('select#' + id).append("<option value=\"" + index + "\">" + value + "</option>");
  });
  
  selectArray.push(id);

  $('select#' + id).change(function() {
    if (callback !== undefined) callback(this.selectedIndex);
  });
}

function radios(label, id, lists, callback)
{
  if (label.length) {
    $('#' + id).append("<td align=\"left\" class=\"style6\" height=\"30\" >" + label + "</td>");
    $('#' + id).append("<td align=\"left\" class=\"style6-2\" id=\"" + id + "\"><td>");
  }
  else {
    $('#' + id).append("<td align=\"left\" class=\"style6\" id=\"" + id + "\"><td>");
  }
  
  $.each(lists, function(index, value) {
    $('td#' + id).append("<input type=\"radio\" value=\"" + index + "\" name=\"" 
	                  + id + "\" id=\"" + id + "\">" + value + "&nbsp;&nbsp;&nbsp;");
  });
  
  $('input#' + id).change(function() {
    if (callback !== undefined && callback !== null) callback(this.value);
  });
}

function checkbox(label, id, value, callback)
{
	if(label !== undefined){
    $('#' + id).append("<td align=\"left\" class=\"style6\" height=\"30\" >" + label + "</td>");
    $('#' + id).append("<td align=\"left\" class=\"style6-2\"><input type=\"checkbox\" id=\"" 
          + id + "\"><span tkey=\"WirelessMask\">"+value+"</span></input></td>");
  }else{
  	$('#' + id).append("<td align=\"left\" class=\"style6\"><input type=\"checkbox\" id=\"" 
          + id + "\"><span tkey=\"WirelessMask\">"+value+"</span></input></td>");
  }
  $('input#' + id).click(function() {
    if (callback !== undefined) callback(this.checked);
  });
}

function addDatalist(id, datalistId)
{
  var datalist;
  
  $(id).attr("list", datalistId);
  datalist = document.createElement("datalist");
  datalist.setAttribute("id", datalistId);
  $(id).parent().append(datalist);
}

function newTable(id, width)
{
  var table = '', tableAttr = '';
  
  if (typeof width !== "undefined")
    tableAttr += 'width="' + width + '" ';
  else
    tableAttr += 'width="70%"';
  
  table += '<td colspan="2" align="left" style="padding: 0px 0px 0px 20px">';
  table += '<table ' + tableAttr + 'border="0" cellpadding="3" cellspacing="1" class="pixeltable" id="' + id + '">';
  table += '<thead id="' + id + 'Head">';
  table += '</thead>';
  table += '<tbody id="' + id + 'Body">';
  table += '</tbody>';
  table += '</table></td>';
  
  return table;
}

function newTableHead(items)
{
  var tableHead = '';
  
  tableHead += '<tr>';
  $.each(items, function(key, data) {
    var attributes = '';
    
    if (typeof data.colspan !== "undefined")
      attributes += 'colspan="' + data.colspan + '" ';
    
    if (typeof data.width !== "undefined")
      attributes += 'width="' + data.width + '" ';
    
    if (typeof data.align !== "undefined")
      attributes += 'align="' + data.align + '" ';
    else
      attributes += 'align="center" ';
    
    if (typeof data.bgcolor !== "undefined")
      attributes += 'bgcolor="' + data.bgcolor + '" ';
    else
      attributes += 'bgcolor="#666666" ';
    
    tableHead += '<th ' + attributes + '><h5>' + data.desc + '</h5></th>'
  });
  tableHead += '</tr>';
  
  return tableHead;
}

function newTableBody(items)
{
  var tableBody = '';
  
  tableBody += '<tr>';
  $.each(items, function(key, data) {
    var attributes = '';
    
    if (typeof data.colspan !== "undefined")
      attributes += 'colspan="' + data.colspan + '" ';
    
    if (typeof data.id !== "undefined")
      attributes += 'id="' + data.id + '" ';
    
    if (typeof data.width !== "undefined")
      attributes += 'width="' + data.width + '" ';
    
    if (typeof data.classes !== "undefined")
      attributes += 'class="' + data.classes + '" ';
    else
      attributes += 'class="style5" ';
    
    if (typeof data.align !== "undefined")
      attributes += 'align="' + data.align + '" ';
    else
      attributes += 'align="center" ';
    
    if (typeof data.bgcolor !== "undefined")
      attributes += 'bgcolor="' + data.bgcolor + '" ';
    
	if (typeof data.wordBreakAll !== "undefined")
      attributes += 'style="word-break:break-all;" ';
    
    tableBody += '<td ' + attributes + '>';
    if (typeof data.isInput !== "undefined") {
      var inputAttr = '';
      if (typeof data.maxSize !== "undefined")
        inputAttr += 'maxlength="' + data.maxSize + '" ';
      
      if (typeof data.inputWidth !== "undefined")
        inputAttr += 'style="width:' + data.inputWidth + '" ';
      
      if (typeof data.name !== "undefined")
        inputAttr += 'name="' + data.name + '" ';
      
      tableBody += '<input type="text" ' + inputAttr + '>';
    }
    else if (typeof data.isDualInput !== "undefined") {
      var inputAttr1 = '', inputAttr2 = '';
      if (typeof data.maxSize !== "undefined") {
        inputAttr1 += 'maxlength="' + data.maxSize + '" ';
        inputAttr2 += 'maxlength="' + data.maxSize + '" ';
      }
      
      if (typeof data.inputWidth !== "undefined") {
        inputAttr1 += 'style="width:' + data.inputWidth + '" ';
        inputAttr2 += 'style="width:' + data.inputWidth + '" ';
      }
      
      if (typeof data.inputSize !== "undefined") {
        inputAttr1 += 'size=' + data.inputSize + '" ';
        inputAttr2 += 'size=' + data.inputSize + '" ';
      }
      
      if (typeof data.name !== "undefined") {
        inputAttr1 += 'name="' + data.name + '1" ';
        inputAttr2 += 'name="' + data.name + '2" ';
      }
      
      tableBody += '<input type="text" ' + inputAttr1 + '>&nbsp;';
      if (typeof data.separator !== "undefined") {
        tableBody += data.separator + '&nbsp;';
      }
	  tableBody += '<input type="text" ' + inputAttr2 + '>';
    }
    else if (typeof data.isImage !== "undefined"){
      tableBody += '<img src="' + data.imgSrc + '" width="18" height="18"/>';
    }
    else if (typeof data.isSelect !== "undefined"){
      var selectAttr = '';
      if (typeof data.id !== "undefined")
        selectAttr += 'id="' + data.id + '" ';
      
      if (typeof data.align !== "undefined")
        selectAttr += 'align="' + data.align + '" ';
      
      tableBody += '<select ' + selectAttr + '>';
      
      if (typeof data.optionList !== "undefined") {
        selectAttr += '<option>="' + data.align + '" </option>';
        $.each(data.optionList, function(index, value) {
          tableBody += "<option value=\"" + index + "\">" + value + "</option>";
        });
      }
      
      tableBody += '</select>';
    }
    else {
      if (typeof data.value !== "undefined")
        tableBody += data.value;
    }
    
    tableBody += '</td>';
  });
  tableBody += '</tr>';
  
  return tableBody;
}

function tableList(id, head, body, width)
{
  var table, tableHead, tableBody;
  
  $('#' + id).append(newTable(id + 'Table', width));
  table = $('#' + id + 'Table');
  $('#' + id + 'TableHead').append(newTableHead(head));
  $('#' + id + 'TableBody').append(newTableBody(body));
}

function font(label, id, width)
{
  var localWidth;
  
  if(width !== undefined) {
    localWidth = width;
  }
  else {
    localWidth = 35;
  }
  
  $('#' + id).append("<td width=\"" + localWidth + "%\" align=\"left\" bgcolor=\"#6A6A6A\" class=\"style3\">&nbsp;" + label + "</td>");
  $('#' + id).append("<td width=\"" + (100-localWidth) + "%\" align=\"left\" bgcolor=\"#6A6A6A\" class=\"style3\"><font color=\"#FFCC00\" \
          style=\"font-weight:bold\" size=\"2\" id=\"" + id + "\"></font></td>");
  
  fontArray.push(id);
}

function doReboot()
{
  var urls;
  var opMode = getOpMode();
  
  switch(opMode) {
    case 1:
      urls = "/f/rp/doReboot"
      rebootTimeout = rebootTimeoutOnRepeaterMode;
      break;
    case 2:
      urls = "/f/ap/doReboot"
      break;
    case 3:
      urls = "/f/br/doReboot"
      break;
    case 0:
    default:
      urls = "/f/doReboot"
      break;
  }
  
  $.ajax({
    url: urls,
    type: "POST",
    success: function(result) {
      console.log(result);
    }
  });
  
  showWindow("waitWindow");
  //Do not need to change host.
  newHost = "";
  countdownWindow("rebootingDig");
}

function LanCountDown()
{
  szazalek=Math.ceil(((totalTime-remainingTime)/totalTime)*100);
  born=3.4*szazalek;
  
  document.getElementById("szliderbar").style.width=born+'px';
  document.getElementById("precentNumber").innerHTML=szazalek+'%';
  
  if (remainingTime <= 0) {
     var page = "http://"+hostipaddr+hosturl;
     window.open(page, "_self");
  }
  else {
    remainingTime--;
	setTimeout(LanCountDown, 1000);
  }
}

function LancountdownWindow(LanIpaddr, url)
{
  var szazalek;
  var born;
  
  totalTime = remainingTime = rebootTimeout;
  
  $('#rebootingDig').show();
  $('#progressBar').show();
  
  hostipaddr = LanIpaddr;
  hosturl = url;
  
  console.log("LanIpaddr = "+hostipaddr+" "+hosturl);
  
  LanCountDown();
}

function doLanReboot(LanIpaddr, url)
{
  var urls;
  var opMode = getOpMode();
  
  switch(opMode) {
    case 1:
      urls = "/f/rp/doReboot"
      break;
    case 2:
      urls = "/f/ap/doReboot"
      break;
    case 3:
      urls = "/f/br/doReboot"
      break;
    case 0:
    default:
      urls = "/f/doReboot"
      break;
  }
  
  $.ajax({
    url: urls,
    type: "POST",
    success: function(result) {
      console.log(result);
    }
  });
  
  showWindow("waitWindow");
  LancountdownWindow(LanIpaddr, url);
}

function autologout(page)
{
 	   logout("autologout");
// document.location.assign("loginerror.html?=autologout");
}
function doLogout()
{
 
   if(confirm(LogoutWarn1) == true) {
	   logout("success");
		//document.location.assign("loginerror.html?=success");
   }else{
	   refresh();
   }
}
/**/
function eraseCookie(name) {
  var ie = navigator.userAgent.match(/MSIE\s([\d.]+)/),
      ie11 = navigator.userAgent.match(/Trident\/7.0/) && navigator.userAgent.match(/rv:11/),
      ieEDGE = navigator.userAgent.match(/Edge/g),
      ieVer=(ie ? ie[1] : (ie11 ? 11 : (ieEDGE ? 12 : -1)));
  if (ie){
       document.execCommand("ClearAuthenticationCache");
   return;
  }
    document.cookie = name +'=; Path=/; Expires=Thu, 01 Jan 1970 00:00:01 GMT;';
}

function logout(msg)
{
  var urls;
  var opMode = getOpMode();
  
  switch(opMode) {
    case 1:
      urls = "/f/rp/doLogout"
      break;
    case 2:
      urls = "/f/ap/doLogout"
      break;
    case 3:
      urls = "/f/br/doLogout"
      break;
    case 0:
    default:
      urls = "/f/doLogout"
      break;
  }

  $.ajax({
    url: urls,
    type: "POST",
    dataType: "json",
    complete: function(result) {
      eraseCookie("x10_session");
      eraseCookie("user");
      console.log(document.cookie);
      
      document.location.assign("loginerror.html?="+msg);
		}
  });
}

var disableHomeButton = 0;
function skipWizard()
{
  $.ajax({
    url:'/f/wizardSetDefault',
    type: "POST",
    success: function(result) {
      goToHomePage();
    },
    error : function() {
      console.log("post /f/wizardSetDefault error");
    }
  });
}

function goToHomePage()
{
  var opMode = getOpMode();
  switch(opMode) {
    case 1:
      window.open("http://"+window.location.host+"/rp", "_self");
      break;
    case 2:
      window.open("http://"+window.location.host+"/ap", "_self");
      break;
    case 2:
      window.open("http://"+window.location.host+"/br", "_self");
      break;
    case 0:
    default:
      window.open("http://"+window.location.host, "_self");
      break;
  }
}

function doGoToDashboard()
{
  var currentPage;
  
  if (disableHomeButton == 1) {
    return;
  }
  
  currentPage = window.location.toString();
  if (currentPage.indexOf("wizard") > -1) {
    if (confirm(SetupWarn1)) {
      skipWizard();
    }
    return;
  }
  
  goToHomePage();
}

function openSupportPage()
{
  window.open("http://www.asrock.com/Networking/X10/", "_blank");
  MM_swapImage('Image12', '', 'images/UI-new_05.jpg', 1);
}

function doChangeLanguage()
{
  //TODO
}

$(document).ready(function() {
  /* handle menu click event */
  $('.menu').click(function(){
    var submenu = $(this).find('.submenu');
    var mainsubmenu = $(this).find('.mainsubmenu');
    if(submenu.length==0){
      var get_url = $(this).find('a').attr('href');
      window.location.assign(get_url);
      return false;
    }else{
      $('.submenu').addClass('close_menu');
      $(submenu).removeClass('close_menu');
      $('.mainsubmenu').addClass('close_menu');
      $(mainsubmenu).removeClass('close_menu');
      $('.close_menu').slideUp();
      //$(submenu).slideDown();
      $(submenu).animate({
        'height': 'toggle', 'opacity': 'toggle'
        }, { duration: 'fast' });
      $(mainsubmenu).animate({
      'height': 'toggle', 'opacity': 'toggle'
      }, { duration: 'fast' });
    }
    return false;
  });
  $('.submenu').click(function(){
    var get_url = $(this).find('a').attr('href');
    window.location.assign(get_url);
    return false;
  });

  /* handle header click event */
  $('#Image10').click(function(){
    doReboot();
  });
  $('#Image11').click(function(){
    doLogout();
  });
  $('#Image12').click(function(){
    openSupportPage();
  });
  $('#Image13').click(function(){
    doGoToDashboard();
  });
  $('#changeLanguage').click(function(){
    doChangeLanguage();
  });
  $.ajaxSetup({
    async: false
  });
  showMultiLang();//langurage check and set.
  $.ajaxSetup({
    async: true
  });

  var urls;
  var opMode = getOpMode();
  switch(opMode) {
    case 1:
      urls = "/f/rp/wizardGetWireless"
      break;
    case 2:
      urls = "/f/ap/wizardGetWireless"
      break;
    case 3:
      urls = "/f/br/wizardGetWireless"
      break;
    case 0:
    default:
      urls = "/f/wizardGetWireless"
      break;
  }
  $.ajaxSetup({
    async: false
  });
  $.ajax({
    url: urls,
    type: "GET",
    dataType: "json",
    timeout: 3000,
    success: function(result) {
      requiredData.ssid2g = decodeURIComponent(result.ssid2g);
      requiredData.psk2g = decodeURIComponent(result.psk2g);
      requiredData.ssid5g = decodeURIComponent(result.ssid5g);
      requiredData.psk5g = decodeURIComponent(result.psk5g);
    },
    error : function() {
      console.log("get" + urls + "error");
      wirelessData = null;
    }
  });
  $.ajaxSetup({
    async: true
  });

  $('#reCheckOnline').click(function(event) {
    console.log("Check online again!");
    checkOnline(1);
  });
});

function changeOnOffStat(switchThis)
{
  var imageOff = document.createElement("img");
  imageOff.src = "images/botton03-off.png";
  imageOff.id = "imageOff";
  imageOff.style.width = "68px";
  imageOff.style.height = "30px";
  var imageOn = document.createElement("img");
  imageOn.src = "images/botton03-on.png";
  imageOn.id = "imageOn";
  imageOn.style.width = "68px";
  imageOn.style.height = "30px";
  var c = switchThis.children;
  if(c[0].id=="imageOn"){
    switchThis.replaceChild(imageOff,c[0]);
    document.getElementById(switchThis.id).value="OFF"
  }
  else{
    switchThis.replaceChild(imageOn,c[0]);
    document.getElementById(switchThis.id).value="ON"
  }
}

function changePassStat(id, maskChecked)
{
  var ieVersion;
  
  ieVersion = msIeVersion();
  if (ieVersion == 0 || ieVersion >= 9) {
    if (maskChecked)
      $('input#' + id).attr('type', 'password');
    else
      $('input#' + id).attr('type', 'text');
  }
}

function writeTitle()
{
  document.write('<title>ASRock X10 Router</title>');
}

function writeApplyCancel()
{
  document.write('<tr><td>&nbsp;</td></tr> \
                  <tr><td>&nbsp;</td></tr> \
                  <tr> \
                    <td colspan="2" align="center"><table border="0" cellspacing="0" cellpadding="0"> \
                      <tr> \
                        <td><table width="119" height="39" border="0" cellspacing="0" cellpadding="0"> \
                          <tr> \
                            <td class="button1" id="Apply"><a>' + DashApply + '</a></td> \
                          </tr> \
                        </table></td> \
                        <td>&nbsp;</td> \
                        <td><table width="119" height="39" border="0" cellspacing="0" cellpadding="0"> \
                          <tr> \
                            <td class="button1" id="Cancel"><a>' + DashCancel + '</a></td> \
                          </tr> \
                        </table></td> \
                      </tr> \
                    </table></td> \
                  </tr>');
}

function writeHeader()
{
  document.write('<tr> \
                    <td width="960" height="113" valign="top"> \
                      <table width="100%" border="0" cellspacing="0" cellpadding="0"> \
                        <tr> \
                          <td><img src="images/UI-new_01.png" width="960" height="7" /></td> \
                        </tr> \
                        <tr> \
                          <td> \
                            <table border="0" cellspacing="0" cellpadding="0"> \
                              <tr> \
                                <td background="images/UI-new_02.png" width="618" height="42"> \
                                  <table width="100%" border="0" cellspacing="0" cellpadding="0"> \
                                    <tr> \
                                      <td width="21%">&nbsp;</td> \
                                      <td width="79%" class="style12">X10</td> \
                                    </tr> \
                                  </table> \
                                </td> \
                                <td> \
                                  <table border="0" cellspacing="0" cellpadding="0"> \
                                    <tr> \
                                      <td><div class="tooltip"><a href="#" onmouseout="MM_swapImage(\'Image10\',\'\',\'images/UI-new_03.jpg\',1)" onmouseover="MM_swapImage(\'Image10\',\'\',\'images/UI-new-over_03.jpg\',1)"><img src="images/UI-new_03.jpg" name="Image10" width="45" height="42" border="0" id="Image10" /></a><span class="tooltiptext">' + GeneralReboot + '</span></div></td> \
                                      <td><div class="tooltip"><a href="#" onmouseout="MM_swapImage(\'Image11\',\'\',\'images/UI-new_04.jpg\',1)" onmouseover="MM_swapImage(\'Image11\',\'\',\'images/UI-new-over_04.jpg\',1)"><img src="images/UI-new_04.jpg" name="Image11" width="40" height="42" border="0" id="Image11" /></a><span class="tooltiptext">' + DashLogout + '</span></div></td> \
                                      <td><div class="tooltip"><a href="#" onmouseout="MM_swapImage(\'Image12\',\'\',\'images/UI-new_05.jpg\',1)" onmouseover="MM_swapImage(\'Image12\',\'\',\'images/UI-new-over_05.jpg\',1)"><img src="images/UI-new_05.jpg" name="Image12" width="40" height="42" border="0" id="Image12" /></a><span class="tooltiptext" id="Image12Span">' + DashHelp + '</span></div></td> \
                                      <td><div class="tooltip"><a href="#" onmouseout="MM_swapImage(\'Image13\',\'\',\'images/UI-new_06.jpg\',1)" onmouseover="MM_swapImage(\'Image13\',\'\',\'images/UI-new-over_06.jpg\',1)"><img src="images/UI-new_06.jpg" name="Image13" width="44" height="42" border="0" id="Image13" /></a><span class="tooltiptext">' + DashHome + '</span></div></td> \
                                    </tr> \
                                  </table></td>\
                                <td><img src="images/UI-new_07.jpg" width="5" height="42" /></td> \
                                <td background="images/UI-new_08.png" width="168" height="42"><table width="100%" border="0" cellspacing="0" cellpadding="0"> \
                                  <tr> \
                                    <td width="17">&nbsp;</td> \
                                    <td width="83%"><script type="text/javascript">writeLanSelect();</script></td> \
                                  </tr> \
                                </table></td> \
                              </tr> \
                            </table> \
                          </td> \
                        </tr> \
                        <tr> \
                          <td><img src="images/UI-new_09.png" width="960" height="64" /></td> \
                        </tr> \
                      </table> \
                    </td> \
                  </tr>');
}

function writeFooter()
{
  document.write('<div class="style9">© 2002-2017 ASRock Inc. All rights reserved. | ' + Notice + '</div>');
}

function writeLeftMenu()
{
  document.write("<div id=\"nav\">");
  document.write("  <ul class=\"menus\">");
  document.write("    <li class=\"menu\"><span><script>document.write(AdvWireless);</script></span>");
  document.write("      <ul id=\"wirelessMenu\">");
  document.write("        <li class=\"submenu\" id=\"wireless2G\"><span><script>document.write(Dash24G);</script></span><a href=\"adv_wlan_2g.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wireless5G\"><span><script>document.write(Dash5G);</script></span><a href=\"adv_wlan_5g.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wirelessGuest\"><span><script>document.write(AdvGuest);</script></span><a href=\"adv_wlan_guest.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wirelessWps\"><span><script>document.write(AdvWPS);</script></span><a href=\"adv_wlan_wps.html\"></a></li>");
  document.write("      </ul>");
  document.write("    </li>");
  document.write("    <li class=\"menu\"><span><script>document.write(AdvLAN);</script></span>");
  document.write("      <ul id=\"lanMenu\">");  
  document.write("        <li class=\"submenu\" id=\"lanLan\"><span><script>document.write(AdvLAN);</script></span><a href=\"adv_lan.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"lanIptv\"><span><script>document.write(AdvIptv);</script></span><a href=\"adv_iptv.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"lanVlan\"><span><script>document.write(AdvlanVlan);</script><a href=\"adv_lan_vlan.html\"></a></span>");
  document.write("      </ul>");
  document.write("    </li>");
  document.write("    <li class=\"menu\"><span><script>document.write(AdvWAN);</script></span>");
  document.write("      <ul id=\"wanMenu\">");
  document.write("        <li class=\"submenu\" id=\"wanInternet\"><span><script>document.write(AdvInternet);</script></span><a href=\"adv_wan_internet.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wanPortTrigger\"><span><script>document.write(AdvPortTrigger);</script></span><a href=\"adv_wan_port_trigger.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wanPortForwarding\"><span><script>document.write(AdvPortForwarding);</script></span><a href=\"adv_wan_port_forwarding.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wanDmz\"><span><script>document.write(AdvDMZ);</script></span><a href=\"adv_wan_dmz.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wanDdns\"><span><script>document.write(AdvDDNS);</script></span><a href=\"adv_wan_ddns.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"wanRemoteManagement\"><span><script>document.write(RemoteManagement);</script></span><a href=\"adv_wan_remote.html\"></a></li>");
  document.write("      </ul>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" id=\"ipv6Menu\"><span align=\"left\"><script>document.write(AdvIPv6);</script></span><a href=\"adv_ipv6.html\"></a>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" id=\"qosMenu\"><span align=\"left\"><script>document.write(AdvQoS);</script></span><a href=\"adv_qos.html\"></a>");
  document.write("    </li>");
  document.write("    <li class=\"menu\"><span><script>document.write(DashVpn);</script></span>");
  document.write("      <ul id=\"vpnMenu\">");
  document.write("        <li class=\"submenu\" id=\"vpnIpsec\"><span><script>document.write(VPNIPSec);</script></span><a href=\"adv_vpn_ipsec.html\"></a></li>");
  document.write("        <li class=\"submenu\" id=\"vpnOpenVpn\"><span><script>document.write(OpenvpnServer);</script></span><a href=\"adv_vpn_openvpn.html\"></a></li>");
  document.write("      </ul>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" id=\"parentControlMenu\"><span align=\"left\"><script>document.write(AdvParentalControl);</script></span><a href=\"adv_parental_control.html\"></a>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" id=\"firewallMenu\"><span align=\"left\"><script>document.write(AdvFirewall);</script></span><a href=\"adv_firewall.html\"></a>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" id=\"usbMenu\"><span align=\"left\"><script>document.write(DashUSB);</script></span><a href=\"adv_USB.html\"></a>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" id=\"homeMenu\"><span align=\"left\"><script>document.write(DashSmartHome);</script></span><a href=\"adv_home_sensors.html\"></a>");
  document.write("    </li>");
  document.write("    <li class=\"menu\" ><span align=\"left\"><script>document.write(AdvAdministration);</script></span>");
  document.write("      <ul id=\"administrationMenu\">");
  document.write("        <li class=\"submenu\" id=\"adminPanel\"><span><script>document.write(AdvRouterPanel);</script></span><a href=\"adv_rt_panel.html\"></a></li>");
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
function writeLanSelect()
{
  document.write("\
    <table width=\"20%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\">\
        <tr>\
        <td width=\"68%\"><form id=\"form1\" name=\"form1\" method=\"post\" action=\"\">\
                      <label for=\"select\"></label>\
                      <select name=\"langSelect\" id=\"langSelect\" class=\"styled-select\" onchange=\"lanFunction(this.value)\">\
                        <option value=\"en\" style=\"background: #666666; color: #FFF;\">English</option>\
                        <option value=\"fr\" style=\"background: #666666; color: #FFF;\">French</option>\
                        <option value=\"it\" style=\"background: #666666; color: #FFF;\">Italian</option>\
                        <option value=\"jp\" style=\"background: #666666; color: #FFF;\">日本語</option>\
                        <option value=\"kr\" style=\"background: #666666; color: #FFF;\">한국어</option>\
                        <option value=\"de\" style=\"background: #666666; color: #FFF;\">Deutsch</option>\
                        <option value=\"es\" style=\"background: #666666; color: #FFF;\">español</option>\
                        <option value=\"si\" style=\"background: #666666; color: #FFF;\">Pусский</option>\
                        <option value=\"pt\" style=\"background: #666666; color: #FFF;\">portuguese</option>\
                        <option value=\"tw\" style=\"background: #666666; color: #FFF;\">繁體中文</option>\
                      </select>\
                      <input type=\"hidden\" name=\"SystemLang\">\
                    </form></td>\
                  </tr>\
                </table>");
}
function isAllNum(str)
{
  for (var i=0; i<str.length; i++) {
    if (str.charAt(i) >= '0' && str.charAt(i) <= '9')
       continue;
    return 0;
  }
  return 1;
}

function isNumOrPeriod(str)
{
  var numOfPeriod=0;

  for (var i=0; i<str.length; i++) {
    if (str.charAt(i) >= '0' && str.charAt(i) <= '9') {
      continue;
    }
    if (str.charAt(i) == '.' ) {
      numOfPeriod++;
      if (numOfPeriod > 3) {
        return 0;
      }
      else {
        continue;
      }
    }
    return 0;
  }
  return 1;
}

function atoi(str, num)
{
  i=1;
  if(num != 1 ){
    while (i != num && str.length != 0){
      if(str.charAt(0) == '.'){
        i++;
      }
      str = str.substring(1);
    }
    if(i != num )
      return -1;
    }

  for(i=0; i<str.length; i++){
    if(str.charAt(i) == '.'){
      str = str.substring(0, i);
      break;
    }
  }
  if(str.length == 0)
    return -1;
  return parseInt(str, 10);
}

function getDigit(str, num)
{
  var i=1;
  if ( num != 1 ) {
        while (i!=num && str.length!=0) {
                if ( str.charAt(0) == '.' ) {
                        i++;
                }
                str = str.substring(1);
        }
        if ( i!=num )
                return -1;
  }
  for (i=0; i<str.length; i++) {
        if ( str.charAt(i) == '.' ) {
                str = str.substring(0, i);
                break;
        }
  }
  if ( str.length == 0)
        return -1;
  var d = parseInt(str, 10);
  return d;
}

function checkRange(str, num, min, max)
{
  d = atoi(str,num);
  if(d > max || d < min)
    return false;
  return true;
}

function checkMask(str, num)
{
  var d = getDigit(str,num);
  if(num == 1){
    if ( ! (d==255))
      return false;
  }
  else if( num == 2 || num == 3){
    if( !(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 || d==255 ))
      return false;
  }
  else if( num == 4 ){
    if( !(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 ))
      return false;
  }
  return true;
}

function checkIPMask(field)
{
  if (field=="") {
    alert(SetupWarn5);
    return false;
  }
  
  if ( isNumOrPeriod( field ) == 0 ) {
    alert(SetupWarn5);
    return false;
  }
  
  if ( !checkMask(field,1) ) {
    alert(SetupWarn5);
    return false;
  }
  
  if ( !checkMask(field,2) ) {
    alert(SetupWarn5);
    return false;
  }
  
  if ( !checkMask(field,3) ) {
    alert(SetupWarn5);
    return false;
  }
  
  if ( !checkMask(field,4) ) {
    alert(SetupWarn5);
    return false;
  }
  
  return true;
}

function checkIPAddr(field)
{
  if(field == ""){
    return false;
  }
  if (isNumOrPeriod(field) == 0) {
    return false;
  }
  if( (!checkRange(field,1,1,223)) ||
    (!checkRange(field,2,0,255)) ||
    (!checkRange(field,3,0,255)) ||
    (!checkRange(field,4,1,254)) ){
    return false;
  }
  return true;
}

function isLocalhost(field)
{
  if(field == "127.0.0.1"){
    return true;
  }
  return false;
}

function ipToLong(ipAddr)
{
  var ipAddrArray, ipAddrValue = 0;

  ipAddrArray = ipAddr.split('.');
  for (var i=0; i<3; i++) {
    ipAddrValue = ipAddrValue + parseInt(ipAddrArray[i]);
    ipAddrValue = ipAddrValue * 256;
  }
  ipAddrValue = ipAddrValue + parseInt(ipAddrArray[3]);

  return ipAddrValue;
}

function isSameSubnet(ipAddr, lanIpAddr, lanNetmask)
{
  var ipAddrValue, lanIpAddrValue, netmaskValue;

  ipAddrValue = ipToLong(ipAddr);
  lanIpAddrValue = ipToLong(lanIpAddr);
  netmaskValue = ipToLong(lanNetmask);

  if ((ipAddrValue & netmaskValue) != (lanIpAddrValue & netmaskValue)) {
    return false;
  }
  else {
    return true;
  }
}

function checkPortNumber(port)
{
  if (port == "" || !isAllNum(port) || parseInt(port) > 65535 || parseInt(port) < 1) {
    return false;
  }
  else {
    return true;
  }
}

function checkIPv6Addr(field) {
  var status = false;
  if (/^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$|^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$|^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$/.test(field)) {
    status = true;
  }
  console.log("checkIPv6Addr "+field+" "+status);
  return status;
}

function getNetmask(field1, field2) {
  return (parseInt(field1) & parseInt(field2));
}

function checkSameSubnet(field1, field2, netmask) {
  var a1 = getNetmask(getDigit(field1, 1), getDigit(netmask, 1));
  var b1 = getNetmask(getDigit(field1, 2), getDigit(netmask, 2));
  var c1 = getNetmask(getDigit(field1, 3), getDigit(netmask, 3));
  var d1 = getNetmask(getDigit(field1, 4), getDigit(netmask, 4));

  var a2 = getNetmask(getDigit(field2, 1), getDigit(netmask, 1));
  var b2 = getNetmask(getDigit(field2, 2), getDigit(netmask, 2));
  var c2 = getNetmask(getDigit(field2, 3), getDigit(netmask, 3));
  var d2 = getNetmask(getDigit(field2, 4), getDigit(netmask, 4));

  if((a1==a2) && (b1==b2) && (c1==c2) && (d1==d2)) {
    return true;
  } else {
    return false;
  }
}

function is4DigitHex(sNum) {
  return /^[0-9A-F]{4}$/i.test(sNum);
}

function isValidDhcpV6Range(start, end) {
  return parseInt(start, 16) < parseInt(end, 16);
}

//Check if all character is ASCII (From space(32) to ~(126))
function checkPrintableChar(str)
{
  if (!(/^[\040-\176]*$/.test(str))) {
    return false;
  }
  return true;
}

function checkSpace(str)
{
  if(str.charAt(0) == ' ' || str.charAt(str.length-1) == ' ') {
    return false;
  }
  else {
    return true;
  }
}

function checkSSID(ssid)
{
  var i = 0, count = 0;
  
  //Can not be NULL
  if (ssid.length == 0) {
    return false;
  }
  
  //Can not be all spaces
  for (i = 0; i < ssid.length; i++) {
    var tempChar = ssid.charCodeAt(i);
    // if the character is a space
    if (tempChar == 32) {
      count++;
    }
  }
  if (count == ssid.length) {
    return false;
  }
  
  //SSID length is 32 at most.
  if (ssid.length > 32) {
    return false;
  }
  
  if (checkPrintableChar(ssid) == false) {
    return false;
  }
  
  return true;
}
function createCookie(name, value, seconds) {
    var expires = "";
    if (seconds) {
        var date = new Date();
        date.setTime(date.getTime() + (seconds * 1000 + 15));
        expires = "; expires=" + date.toGMTString();
    }
    document.cookie = name + "=" + value + expires + "; path=/";
}
function readCookie(name) {
    var nameEQ = name + "=";
    var ca = document.cookie.split(';');
    for (var i = 0; i < ca.length; i++) {
        var c = ca[i];
        while (c.charAt(0) == ' ') c = c.substring(1, c.length);
        if (c.indexOf(nameEQ) === 0) return c.substring(nameEQ.length, c.length);
    }
    return null;
}
function hasCookie(name) {
    return (document.cookie || "").indexOf(name+"=") != -1;
}
function isLoggedIn() {
    return 0;
}
function lanFunction(langVaule)
{
  if(langVaule=="en")
    $.getJSON('lang/en.json', translate);
  else if(langVaule=="jp"){
    $.getJSON('lang/jp.json', translate);
    $('#leftmenu_parental').removeClass('style2');
    $('#leftmenu_parental').addClass('style2-1');
  }
  else if(langVaule=="fr")
    $.getJSON('lang/fr.json', translate);
  else if(langVaule=="it")
    $.getJSON('lang/it.json', translate);
  else if(langVaule=="kr")
    $.getJSON('lang/kr.json', translate);
  else if(langVaule=="de"){
    $.getJSON('lang/de.json', translate);
    $('#ad1','#ad2','#ad3','#ad4','#ad5','#ad6','#ad7').removeClass('submenu');
    $('#ad1','#ad2','#ad3','#ad4','#ad5','#ad6','#ad7').addClass('submenu1');
  }
  else if(langVaule=="tw")
    $.getJSON('lang/tw.json', translate);
  else if(langVaule=="si")
    $.getJSON('lang/si.json', translate);
  else if(langVaule=="pt")
    $.getJSON('lang/pt.json', translate);
  else if(langVaule=="es")
    $.getJSON('lang/es.json', translate);
  else
    $.getJSON('lang/en.json', translate);

  var formData = {"SystemLang":langVaule};
  
  var urls;
  var opMode = getOpMode();
  
  switch(opMode) {
    case 1:
      urls = "/f/rp/setSysLang"
      break;
    case 2:
      urls = "/f/ap/setSysLang"
      break;
    case 3:
      urls = "/f/br/setSysLang"
      break;
    case 0:
    default:
      urls = "/f/setSysLang"
      break;
  }
  
  $.ajax ({
    url: urls,
    type: 'POST',
    data: formData,
    dataType: 'json',
    async:false,
    success:function(data) {
      if(data.status==0) {
        $.ajax ({
          url: '',
          type: "GET",
          dataType: 'json',
          async:false,
          success:function(data) {
            document.location.reload(true);
          }
        });
        // return true;
      }
    }
  });
            document.location.reload(true);
}

var translate = function (jsdata)
{
	console.log("in translate function");
  $("[tkey]").each (function (index)
  {
    var strTkey = jsdata [$(this).attr ('tkey')];
	console.log("tkey= %s, jsdata=%s\n",$("[tkey]"),strTkey);
    if (strTkey != null){
      // var strData = strTkey.replace("\\", "");
        $(this).html (strTkey);
    }
  });

}
function showMultiLang()
{
  var urls;
  var opMode = getOpMode();
  
  switch(opMode) {
    case 1:
      urls = "/f/rp/getSysLang"
      break;
    case 2:
      urls = "/f/ap/getSysLang"
      break;
    case 3:
      urls = "/f/br/getSysLang"
      break;
    case 0:
    default:
      urls = "/f/getSysLang"
      break;
  }
  
 $.ajax
  ({
    url: urls,
    type: "GET",
    dataType: 'json',
    success:function(data)
    {
      if (data['SystemLang'] == "en"){
        document.getElementById("langSelect").selectedIndex=0;
        $.getJSON('lang/en.json', translate);
      }
      else if (data['SystemLang'] == "fr"){
        document.getElementById("langSelect").selectedIndex=1;
        $.getJSON('lang/fr.json', translate);
      }
      else if (data['SystemLang'] == "it"){
        document.getElementById("langSelect").selectedIndex=2;
        $.getJSON('lang/it.json', translate);
      }
      else if (data['SystemLang'] == "jp"){
        document.getElementById("langSelect").selectedIndex=3;
        $.getJSON('lang/jp.json', translate);
        $('#leftmenu_parental').removeClass('style2');
        $('#leftmenu_parental').addClass('style2-1');
      }
      else if (data['SystemLang'] == "kr"){
        document.getElementById("langSelect").selectedIndex=4;
        $.getJSON('lang/kr.json', translate);
      }
      else if (data['SystemLang'] == "de"){
        document.getElementById("langSelect").selectedIndex=5;
        $.getJSON('lang/de.json', translate);
      }
      else if (data['SystemLang'] == "es"){
        document.getElementById("langSelect").selectedIndex=6;
        $.getJSON('lang/es.json', translate);
      }
      else if (data['SystemLang'] == "si"){
        document.getElementById("langSelect").selectedIndex=7;
        $.getJSON('lang/si.json', translate);
      }
      else if (data['SystemLang'] == "pt"){
        document.getElementById("langSelect").selectedIndex=8;
        $.getJSON('lang/pt.json', translate);
      }
      else if (data['SystemLang'] == "tw"){
        document.getElementById("langSelect").selectedIndex=9;
        $.getJSON('lang/tw.json', translate);
      }
      else{
        document.getElementById("langSelect").selectedIndex=0;
        $.getJSON('lang/en.json', translate);
      }
	console.log("langurage is %s - index is %d ",data['SystemLang'],document.getElementById("langSelect").selectedIndex);

    },
    error : function() {
      console.log("get " + urls + " error");
    }
  });
}

function addBreak(str, step)
{
  var newStr = "";

  for (i = 0; i < str.length; i++) {
    if ( (i > 0) && (i % step == 0)) {
      newStr += "<br />&nbsp;";
    }
    newStr += str.charAt(i);
  }

  return newStr;
}
