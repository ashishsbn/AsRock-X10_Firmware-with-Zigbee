// Define some global vars for our idle timer
var idleTime = 0;
var idleCountdown = 10;
var idlemin = 10;
var idlemax = 300;
var timerStop = 0;

//
//
// you need to define your own HTML and css styles for the warning message and fade layer.
// the warning message goes in a div with the ID #idlewarn
// the css should style for a fade layer with the id #fade
//
//


// N.B. using jQuery - easily adapted for other *.js library.
$(document).ready(function(){
  
  // every N sec this example goes every 1 sec BUT that's a bit much in practice.
   var idleInterval = setInterval("timerIncrement()", 1000); 

    // Zero the idle timer on mouse movement.
    $(window).mousedown(function (event) {
        // hide warning + idleCountdown
        idleCountdown = 10;
        idleTime = 0;
    });
    // Also Zero the timer on keypress
    $(window).keydown(function (event) {
        // hide warning + idleCountdown
        idleCountdown = 10;
        idleTime = 0;
    });

}); // end document.ready block


// The function that gets called every second.
function timerIncrement() {
    idleTime ++; 

    // if user has been idle for the idlemin time do this - show warning message.
    if (idleTime > idlemax && timerStop == 0) { 
	  autologout("autologout");
//	  clearTimeout(idleInterval);

    }

}//end timer increment