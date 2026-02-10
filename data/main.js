window.addEventListener('DOMContentLoaded', function() {
    // 1) Load the current network settings from the ESP
    loadNetworkSettings();

    // 2) Setup the form submission listener
    const form = document.getElementById('networkForm');
    form.addEventListener('submit', function(event) {
        event.preventDefault();

        // Show "Settings submitted" text
        document.getElementById('submitMessage').style.display = 'block';
        const isChecked = document.getElementById('dhcp').checked;
        // Then do the actual update (POST) with the function in api.js
        if(isChecked){
            updateNetworkSettings(
                "0.0.0.0",
                "0.0.0.0",
                "0.0.0.0",
                document.getElementById('hostname').value
            );
        }
        else {
            updateNetworkSettings(
                document.getElementById('ip').value,
                document.getElementById('gateway').value,
                document.getElementById('subnet').value,
                document.getElementById('hostname').value
            );
        }

    });

    document.getElementById('resetNetworkBtn').addEventListener('click', function() {
        resetNetworkSettings();
    });

    document.getElementById('dhcp').addEventListener('change', toggleDHCP);
});

/**
 * Fetches the network settings from the ESP32 (our new /getNetworkSettings route),
 * then updates the form fields accordingly.
 */
function loadNetworkSettings() {
    fetch("/getNetworkSettings")
        .then(response => {
            if (!response.ok) {
                throw new Error("Failed to fetch network settings");
            }
            return response.json();
        })
        .then(data => {

            document.getElementById('ip').value       = data.ip       || "";
            document.getElementById('gateway').value  = data.gateway  || "";
            document.getElementById('subnet').value   = data.subnet   || "";
            document.getElementById('hostname').value = data.hostname || "";

            document.getElementById('dhcp').checked = data.dhcp === true;
            toggleDHCP();
        })
        .catch(err => {
            console.error("Error while fetching network settings:", err);
        });
}