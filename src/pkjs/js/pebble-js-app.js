Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://randa.xyz/pebble_config/index.html';

  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));

  console.log('Configuration page returned: ' + JSON.stringify(configData));

  if (configData.starting_Time) {
    Pebble.sendAppMessage({
      START_TIME_IN: parseInt(configData.starting_Time, 10),
      END_TIME_IN: parseInt(configData.ending_Time, 10)
    }, function() {
		console.log('Send successful!: ' + JSON.stringify(configData));
    }, function() {
      console.log('Send failed!');
    });
  }
});