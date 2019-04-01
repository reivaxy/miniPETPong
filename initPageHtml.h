/**
 *  boostrap init page 
 *  Xavier Grosjean 2017
 *  Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International Public License
 */

#include "messages.h"
 
// no empty line please

char initPage[] = "\
<html title='bootstram'>\
<body>\
<h1>" MSG_INIT_WELCOME "</h1>\
<form action='/initSave' method='post'>\
  <input name='APSsid' type='text' placeholder='" MSG_INIT_AP_SSID "'/><br/>\
  <input name='APPwd' type='text' placeholder='" MSG_INIT_AP_PWD "'/><br/>\
  <input name='homeSsid' type='text' placeholder='" MSG_INIT_HOME_SSID "'/><br/>\
  <input name='homePwd' type='text' placeholder='" MSG_INIT_HOME_PWD "'/><br/>\
  <input type='submit'/>\
</form>\
</body>\
</html>\
";
