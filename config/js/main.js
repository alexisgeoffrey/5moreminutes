(function() {
  loadOptions();
  submitHandler();
})();

function submitHandler() {
  var $submit_button = ('$submit_button');
  
  $submit_button.on('click', function() {
    console.log('Submit');
    
    var return_to = getQueryParam('return_to', 'pebblejs://close#');
    document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfigData()));
  });
}
  
function loadOptions() {
  var $startTime = $('#start_time_text');
  var $endTime = $('#end_time_text');
  
  if (localStorage.start_Time) {
    $startTime[0].value = localStorage.start_Time;
    $endTime[0].value = localStorage.end_Time;
  }
}

function getAndStoreConfigData() {
  var $startTime = $('#start_time_text');
  var $endTime = $('#end_time_text');
  
  var options = {
    start_Time: $startTime.val(),
    end_Time: $endTime.val()
  };
  
  localStorage.startTime = options.startTime;
  localStorage.endTime = options.endTime;
  
  console.log('Got options: ' + JSON.stringify(options));
  return options;
}

function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}
