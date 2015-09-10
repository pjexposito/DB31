Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://dl.dropboxusercontent.com/u/119376/config-time/index.html';
  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));


  
  var dict = {};

    dict['KEY_IDIOMA'] = configData.idioma;
    dict['KEY_VIBE'] = configData.vibe;
    dict['KEY_DATEFORMAT'] = configData.dateformat;
    dict['KEY_SEGUNDOS'] = configData.segundos;
    dict['KEY_HOURLYVIBE'] = configData.hourlyvibe;
    dict['KEY_BACK'] = configData.back;
console.log(JSON.stringify(dict));
  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!: '+e.error.message);
  });
});

/*
// El código no es mio. Está sacado de https://github.com/C-D-Lewis/pebble-sdk2-tut-9

Pebble.addEventListener("ready",
  function(e) {
    //console.log("Pebblejs cargado");
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
    //Carga la web de configuración
    Pebble.openURL("https://dl.dropboxusercontent.com/u/119376/config-time.html");
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));
    //console.log("Datos devueltos: " + JSON.stringify(configuration));
 
    //Send to Pebble, persist there
    Pebble.sendAppMessage(
      {"KEY_IDIOMA": configuration.idioma,
       "KEY_VIBE": configuration.vibe,
       "KEY_DATEFORMAT": configuration.dateformat,
       "KEY_SEGUNDOS": configuration.segundos,
       "KEY_HOURLYVIBE": configuration.hourlyvibe,
       "KEY_BACK": configuration.back
      },
      function(e) {
        //console.log("Mandando datos...");
      },
      function(e) {
        //console.log("Error al mandar datos");
      }
    );
  }
);


*/