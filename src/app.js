
Pebble.addEventListener('ready', function() {
  	console.log('PebbleKit JS ready!'); 
	if(localStorage.getItem("wp_token") != null){
		wordpressData();
	}
});

Pebble.addEventListener('showConfiguration', function() {
	config();
});

function config(){
	//if(localStorage.getItem("wp_token") == null){
		console.log("Opening WordPress OAuth..");
		Pebble.openURL('https://public-api.wordpress.com/oauth2/authorize?client_id=46347&redirect_uri=http%3A%2F%2Ficodethings.info%2Fpebble-redirect.php&response_type=code');
	//}else{
	//	console.log("Token: " + localStorage.getItem("wp_token"));
	//}
}

function wordpressAuth(e){
	console.log("Oauth closed: " + JSON.stringify(e));
	var decoded = decodeURIComponent(e.response);
	if(decoded != undefined){
		console.log("Decoded:"+decoded);
		var configuration;
		try {
			configuration = JSON.parse(decoded);
			console.log("config: " + configuration);
		} catch(err) {
			console.log("Failed to parse:"+err);
			// no configuration needed?
			// if (localStorage.getItem("wp_token") && !loadedInit) {
			//   console.log("token available, okay...");
			//loadBoards();
			//  }
			return;
		}

		console.log("fetching full token...");

		var xhttp = new XMLHttpRequest();
		xhttp.onreadystatechange = function() {
			if (xhttp.readyState == 4 && xhttp.status == 200) {
				console.log("OAuth info: " + xhttp.responseText);
				var json = JSON.parse(xhttp.responseText);
				var access_token = json['access_token'];
				var blog_id = json['blog_id'];
				var blog_url = json['blog_url'];
				localStorage.setItem("wp_token", access_token);
				localStorage.setItem("wp_id", blog_id);
				localStorage.setItem("wp_url", blog_url);
				wordpressData();
			}
		};
		xhttp.open("POST", "https://public-api.wordpress.com/oauth2/token", false);
		xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
		xhttp.send("client_id=46347&redirect_uri=http%3A%2F%2Ficodethings.info%2Fpebble-redirect.php&client_secret=OvbEkrx8R8rB9X0R1hUqWyo9UOnhVbCZroERNpKigolIL1RoykhLvMVZ0pwdR2gV&code=" + configuration.token + "&grant_type=authorization_code");
	}
}

Pebble.addEventListener('webviewclosed', function(e) {
	wordpressAuth(e);
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    wordpressData();
  }                     
);

function wordpressData(){
	if(localStorage.getItem("wp_token") !== null){
		console.log("token exists, getting stats...");
		var xhttpStats = new XMLHttpRequest();
		xhttpStats.onreadystatechange = function() {
			if (xhttpStats.readyState == XMLHttpRequest.DONE && xhttpStats.status == 200) {
				console.log("got stats, assembling json..");
				var json = JSON.parse(xhttpStats.responseText);
				
				console.log("got stats, assembling views..");
				var views = [];
				var length = 30;
				//var maxViews = 144;
				//var length = maxViews;
				
				if(length > json.visits.data.length)
					length = json.visits.data.length;
				
				console.log(length + " days of views");
				
				for(var i = 0; i < length; ++i){
					views[i] = json["visits"]["data"][i][1];
				}
				
				// Send to Pebble
				Pebble.sendAppMessage(
					{
						day_views_count: json['stats']['views_today'],
						views_best: json['stats']['views_best_day_total'],
						day_views: views
						
					},
					function(e) {
						console.log("Wordpress stat info sent to Pebble successfully!");
					},
					function(e) {
						console.log("Error sending stats info to Pebble!");
					}
				);
			}
		};
		xhttpStats.open("GET", "https://public-api.wordpress.com/rest/v1.1/sites/"+encodeURIComponent(localStorage.getItem("wp_id"))+"/stats", true);
		xhttpStats.setRequestHeader("authorization", "Bearer " + localStorage.getItem("wp_token"));
		xhttpStats.send();
	}
}
						
/*var myAPIKey = '';

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
      pos.coords.latitude + "&lon=" + pos.coords.longitude + '&appid=' + myAPIKey;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log("Conditions are " + conditions);
      
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_CONDITIONS": conditions
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getWeather();
  }                     
);*/
