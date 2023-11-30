<!DOCTYPE html>
<html class="dark-mode">
  <head>
    <meta charset="utf-8">
    <title>HFOP Navigation</title>
    <script src="https://polyfill.io/v3/polyfill.min.js?features=default"></script>
    <link rel="stylesheet" type="text/css" href="./style.css" />
    <!-- UIkit CSS -->
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/uikit@3.9.2/dist/css/uikit.min.css" />
    <style>
      .container {
       display: flex;
       flex-wrap: wrap;
       align-items: center;
      }
      #save_type {
       flex: 1;
      }
      #pac-input {
       flex: 2;
      }
      .coords-container {
        width: 100%;
      } 
      .dark-mode {
  background-color: #121212; /* Dark background color */

  color: #ffffff; /* Light text color */
      }
    </style>    
    <!-- UIkit JS -->
    <script src="https://cdn.jsdelivr.net/npm/uikit@3.9.2/dist/js/uikit.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/uikit@3.9.2/dist/js/uikit-icons.min.js"></script>

    <script src="./index.js"></script>
    <meta name="viewport" content="width=device-width">
  </head>
<body>
    <div>選擇類別後所尋找的地點會儲存在導航介面的選單內，需重新啟動設備才會顯示</div>
    <div class="container">
        <select id="save_type" class="uk-select" name="type">
          <option value="recent">最近</option>
          <option value="home">家</option>
          <option value="work">公司</option>
        </select>      
        <input class="uk-input" type="text" id="pac-input" name="keyword" placeholder="尋找地點" onfocus='this.value=""'/>
    </div>
    <div class="coords-container">
    <input class="uk-input" type="text" id="coords" readonly />
    </div>
    <div style="display: flex;">

  <div style="flex: 1;">
    <div id="destinationHeading"></div>
    <div id="jsonOutput"></div>
  </div>

  <div id="map" style="flex: 0 0 33.33%;"></div>

</div>
    
    <!-- Async script executes immediately and must be after any DOM elements used in callback. -->
    <script
      src="https://maps.googleapis.com/maps/api/js?key={{gmap_key}}&callback=initAutocomplete&libraries=places&v=weekly&language={{language}}"
      async
    ></script>
  </body>
</html>
