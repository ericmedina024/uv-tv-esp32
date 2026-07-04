Config hostConfigurationServer(const Config &currentConfig);

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>UV TV Config</title>
  </head>
  <body>
    <h1>UV TV Config</h1>

    <p id="error-message" style="color:red;"></p>

    <form method="post">
      <label for="wifi_network">WiFi Network</label>
      <input type="text" id="wifi_network" name="wifi_network" required>
      <br>

      <label for="wifi_password">WiFi Password</label>
      <input type="text" id="wifi_password" name="wifi_password" required>
      <br>

      <label for="latitude">Latitude</label>
      <input type="number" step="any" id="latitude" name="latitude" required>
      <br>

      <label for="longitude">Longitude</label>
      <input type="number" step="any" id="longitude" name="longitude" required>
      <br>

      <button type="submit">Submit</button>
      
    </form>

    <script>
      (async () => {
        const errorMessageElement = document.getElementById("error-message");
        const returnedError = new URLSearchParams(location.search).get("error");
        if (returnedError) {
          errorMessageElement.textContent = returnedError;
        }
        const currentConfig = await (await fetch("/current-config")).json();
        document.getElementById("longitude").value = currentConfig["longitude"];
        document.getElementById("latitude").value = currentConfig["latitude"];
        document.getElementById("wifi_network").value = currentConfig["wifiSsid"];
        document.getElementById("wifi_password").value = currentConfig["wifiPassword"];
      })();
    </script>

  </body>
</html>
)HTML";


const char SUCCESS_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <title>UV TV Config</title>
  </head>
  <body>
    <h1>Successfully updated configuration!</h1>
  </body>
</html>
)HTML";
