// This example adds a search box to a map, using the Google Place Autocomplete
// feature. People can enter geographical searches. The search box will return a
// pick list containing a mix of places and predicted search terms.
// This example requires the Places library. Include the libraries=places
// parameter when you first load the API. For example:
// <script src="https://maps.googleapis.com/maps/api/js?key=YOUR_API_KEY&libraries=places">
function initAutocomplete() {
  const map = new google.maps.Map(document.getElementById("map"), {
    center: { lat: {{lat}}, lng: {{lon}} },
    zoom: 13,
    mapTypeId: "roadmap",
    disableDefaultUI: true
  });
  // Create the search box and link it to the UI element.
  const input = document.getElementById("pac-input");
  const searchBox = new google.maps.places.SearchBox(input);

  // Bias the SearchBox results towards current map's viewport.
  map.addListener("bounds_changed", () => {
    searchBox.setBounds(map.getBounds());
  });

  let markers = [];

  // Listen for the event fired when the user selects a prediction and retrieve
  // more details for that place.
  searchBox.addListener("places_changed", () => {
    const places = searchBox.getPlaces();

    if (places.length == 0) {
      return;
    }

    // Clear out the old markers.
    markers.forEach((marker) => {
      marker.setMap(null);
    });
    markers = [];

    // For each place, get the icon, name and location.
    const bounds = new google.maps.LatLngBounds();

    places.slice(-1).forEach((place) => {
      if (!place.geometry || !place.geometry.location) {
        console.log("Returned place contains no geometry");
        return;
      }

      const icon = {
        url: place.icon,
        size: new google.maps.Size(71, 71),
        origin: new google.maps.Point(0, 0),
        anchor: new google.maps.Point(17, 34),
        scaledSize: new google.maps.Size(25, 25),
      };

      // Create a marker for each place.
      markers.push(
        new google.maps.Marker({
          map,
          icon,
          title: place.name,
          position: place.geometry.location,
        })
      );

      // set nav
      var http = new XMLHttpRequest();
      http.open("POST", "/", true);
      http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
      var params = "lat=" + place.geometry.location.lat() + "&lon=" + place.geometry.location.lng();
      params += "&save_type=" + document.getElementById("save_type").value;
      params += "&name=" + place.name;
      http.send(params);
      // Update coords input and copy to clipboard
      var coordsInput = document.getElementById("coords");
      coordsInput.value = place.geometry.location.lat() + ", " + place.geometry.location.lng();
      coordsInput.select();
      document.execCommand("copy");    
      return;
    });    
  });  
}

// Fetch the JSON content from navpasedestination.json
fetch('/get_past_dest')
  .then(response => response.json())
  .then(data => {
    const mapDiv = document.getElementById("map");
    let tableContent = "<table>";

    // Add table headers
    tableContent += "<tr>";
    tableContent += "<th>Place Name</th>";
    tableContent += "<th>Save Type</th>";
    tableContent += "</tr>";

    // Iterate over each place in the JSON data
    data.forEach(place => {
      tableContent += "<tr>";
      tableContent += `<td><a href="#" onclick="setNav('${place.place_name}', ${place.latitude}, ${place.longitude}, '${place.save_type}')">${place.place_name}</a></td>`;
      tableContent += `<td>${place.save_type}</td>`;
      tableContent += "</tr>";
    });

    tableContent += "</table>";

    // Set the HTML content of the map div
    mapDiv.innerHTML = tableContent;
  })
  .catch(error => {
    console.log('Error:', error);
  });

function setNav(name, lat, lon, savetype) {
  // set nav
  var http = new XMLHttpRequest();
  http.open("POST", "/", true);
  http.setRequestHeader("Content-type","application/x-www-form-urlencoded");
  var params = "lat=" + lat + "&lon=" + lon;
  params += "&save_type=" + savetype;
  params += "&name=" + name;
  http.send(params);
  // 將place_name的值設定給pac-input輸入框
  document.getElementById("pac-input").value = name;
  // Update coords input and copy to clipboard
  var coordsInput = document.getElementById("coords");
  coordsInput.value = lat + ", " + lon;
  coordsInput.select();
  document.execCommand("copy");
}

let useMetricUnits = false;
  let previousNavdirectionsUuid = null;
  let previousCurrentStepUuid = null;
  let jsonData = null;
  let initNav = 0;

  async function loadCurrentStep() {
    try {
      const response = await fetch('CurrentStep.json'); // Load CurrentStep.json

      if (!response.ok) {
        throw new Error('Failed to fetch CurrentStep.json.');
      }

      const json = await response.json();
      return json;
    } catch (error) {
      console.error('Error fetching or parsing CurrentStep.json:', error);
      return null;
    }
  }

  async function loadNavdirectionsData() {
    try {
      const response = await fetch('navdirections.json'); // Load navdirections.json

      if (!response.ok) {
        throw new Error(`Failed to fetch JSON file. Status: ${response.status}`);
      }

      const json = await response.json();

      // Check if the UUIDs match
      const match = json.uuid === previousCurrentStepUuid;

      previousNavdirectionsUuid = json.uuid;
      jsonData = json;
      initNav = 1;
      return jsonData;
    } catch (error) {
      console.error('Error fetching or parsing JSON data:', error);
      return jsonData; // Return the existing data on error
    }
  }

  async function fetchAndDisplayData() {
    const currentStepData = await loadCurrentStep();

    if (currentStepData !== null) {
      // Set the initial value for `currentStep` based on `CurrentStep.json`
      previousCurrentStepUuid = currentStepData.uuid;
    }

    if (currentStepData.uuid != previousNavdirectionsUuid) {
      await loadNavdirectionsData();
    }

    if (initNav === 0) {
      await loadNavdirectionsData(); 
    }

    // Check if jsonData is available and proceed
    if (jsonData) {
      // Access the data you need from the loaded JSON
      const firstRoute = jsonData.routes[0];
      const firstLeg = firstRoute.legs[0];
      const steps = firstLeg.steps;
      const destination = firstRoute.Destination;

      // Determine whether to use metric or imperial units based on the 'Metric' key
      useMetricUnits = firstRoute.Metric === true; // Removed `const` to update the global useMetricUnits

      // Display the 'destination' value on the webpage
      const destinationHeading = document.getElementById('destinationHeading');
      destinationHeading.textContent = `Destination: ${destination}`;

      // Display values from the steps
      const jsonOutputDiv = document.getElementById('jsonOutput');
      jsonOutputDiv.innerHTML = '';

      for (let i = currentStepData.CurrentStep; i < steps.length - 1; i++) {
        const step = steps[i];
        const instruction0 = steps[i].maneuver.instruction;
        const instruction = steps[i + 1].maneuver.instruction;
        let distance = step.distance;

        if (!useMetricUnits) {
          // Convert distance to miles if using imperial units
          distance = distance * 0.000621371;
        } else {
          distance = distance / 1000; // Convert meters to kilometers
        }

        // Display the values on the webpage
        if (i === 0) {
          jsonOutputDiv.innerHTML += `
            <p>${instruction0}</p>
            <hr>
          `;
        }
        jsonOutputDiv.innerHTML += `
          <p>In ${distance.toFixed(1)} ${useMetricUnits ? 'km' : 'miles'}: ${instruction}</p>
          <hr>
        `;
      }
    }
  }

  // Load `CurrentStep.json` initially
  loadCurrentStep().then((currentStepData) => {
    if (currentStepData !== null) {
      // Set the initial value for `currentStep` based on `CurrentStep.json`
      previousCurrentStepUuid = currentStepData.uuid;
      loadNavdirectionsData();
      // Fetch and display data initially
      fetchAndDisplayData();
    }
  });

  // Periodically fetch `CurrentStep.json` and display data every 5 seconds
  setInterval(fetchAndDisplayData, 5000); // Adjust the interval as needed (in milliseconds)
