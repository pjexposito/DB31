// El código no es mio. Está sacado de https://github.com/C-D-Lewis/pebble-sdk2-tut-9

Pebble.addEventListener("ready",
  function(e) {
    //console.log("Pebblejs cargado");
  }
);

Pebble.addEventListener("showConfiguration",
  function(e) {
    //Carga la web de configuración
    //Pebble.openURL("https://dl.dropboxusercontent.com/u/119376/config-time.html"); // original url: without dutch option
    Pebble.openURL("https://dl.dropboxusercontent.com/u/78618251/DB31/config-time.html");
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